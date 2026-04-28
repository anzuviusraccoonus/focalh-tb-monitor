#include <filesystem>
#include "DataReader.h"
#include "spdlog/spdlog.h"
#include "globals.h"

DataReader* DataReader::mp_instance = nullptr;

DataReader::DataReader() {
    spdlog::info("Initializing DataReader");
    m_target = "";
    Reset();
    mp_parser = new bp::StreamParser(

        // Function to run when we get a generic packet
        [&](const bp::Packet& pkt) {
            (void)pkt;
            std::scoped_lock lock(g_mutex);
            ++num_packets;
        },

        // Function to run when we get a heartbeat packet
        [&](const bp::Heartbeat&) {
            std::scoped_lock lock(g_mutex);
            spdlog::get("Integrity")->debug("[HRT.]");
			++num_heartbeat_packets;
        },
        
        // Function to run when we get a sync packet(?)
        [&](std::span<const std::byte>) {
            std::scoped_lock lock(g_mutex);
            spdlog::get("Integrity")->debug("[SYNC]");
			++num_sync_packets;
        },
        
        // Function to run when we get an RDH L0 packet
        [&](const bp::RDH_L0& rdh, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
            //spdlog::get("Integrity")->debug("[RDH0] {:02X} {:02X} {:04X} {:02X} {:02X} {:04X} {:04X} {:04X} {:02X} {:02X} {:04X} {:02X} {:04X} {:08X} {:08X} {:02X} {:08X} {:08X}",
            //              rdh.header_version,
            //              rdh.header_size,
            //              rdh.fee_id,
            //              rdh.priority_bit,
            //              rdh.system_id,
            //              rdh.reserved0,
            //              rdh.offset_new_packet,
            //              rdh.memory_size,
            //              rdh.link_id,
            //              rdh.packet_counter,
            //              rdh.cru_id,
            //              rdh.dw,
            //              rdh.bc,
            //              rdh.reserved1,
            //              rdh.orbit,
            //              rdh.data_format,
            //              rdh.reserved2,
            //              rdh.reserved3 );
			++l0_lines_read;
        },
        
        // Function to run when we get an RDH L1 packet
        [&](const bp::RDH_L1& rdh, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
            //spdlog::get("Integrity")->debug("[RDH1] {:08X} {:04X} {:02X} {:02X} {:08X} {:08X} {:08X} {:04X} {:04X} {:08X} {:08X}",
            //              rdh.trg_type, 
            //              rdh.hb_packet_counter,
            //              rdh.stop_bit,
            //              rdh.reserved0,
            //              rdh.reserved1,
            //              rdh.reserved2,
            //              rdh.detector_field,
            //              rdh.par_bit,
            //              rdh.reserved3,
            //              rdh.reserved4,
            //              rdh.reserved5 );
			++l1_lines_read;
        },
        
        // Function to run when we get a data line
        [&](const bp::DataLine& line, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
			++data_lines_read;

            // Don't process any data lines unless we got a trigger line first
            if (not m_has_active_trigger[line.header_vldb_id]) { return; }

            // This block determines when we've found the start of a DAQ frame,
            // IMPORTANT: *for a given VLDB link*
	    	if (not m_is_reading_daq_frame[line.header_vldb_id]) {
                    auto magic = 0xF000000F;

                    // to do: fucking fix this ugly piece of shit
                    if ( ((line.data_word0 & magic) == 0xF0000005) ||
                         ((line.data_word1 & magic) == 0xF0000005) ||
                         ((line.data_word2 & magic) == 0xF0000005) ||
                         ((line.data_word3 & magic) == 0xF0000005) ||
                         ((line.data_word0 & magic) == 0xF0000002) ||
                         ((line.data_word1 & magic) == 0xF0000002) ||
                         ((line.data_word2 & magic) == 0xF0000002) ||
                         ((line.data_word3 & magic) == 0xF0000002) ) {

                            OnDAQFrameStart(line);
                    }
            }

            // If we have an active trigger, and found the start of a DAQ frame,
            // we process the line for information and other statistics
            if (m_is_reading_daq_frame[line.header_vldb_id]) {
                
                // Keeping track of bunch crossing counters and accounting for rollover
                if (not (line.bx_cnt == (m_vldb_bx_counter[line.header_vldb_id] + 1))) {
                    if ((line.bx_cnt == 0) & (m_vldb_bx_counter[line.header_vldb_id] == 3563)) {
                        m_vldb_bx_counter[line.header_vldb_id] = -1;
                    }
                }

                buffer_bx_counters.push_back(line.bx_cnt - m_vldb_bx_counter[line.header_vldb_id]);
                m_vldb_bx_counter[line.header_vldb_id] += 1;

                // DAQ header line - we extract some useful information here,
                // but it must be processed differently from regular lines
                if (m_vldb_line_counter[line.header_vldb_id] == 0) {
                    m_words[line.header_vldb_id] = { { line.data_word0, line.data_word1 },
                                                   { line.data_word2, line.data_word3 } };
	    	    	
	    	    	unsigned int hd, bx, ec, ob, ham, tr;
	    	    	for (unsigned int asic = 0; asic < g_NUM_ASIC_PER_VLDB; ++asic) {
	    	    		for (unsigned int half = 0; half < g_NUM_HALVES_PER_ASIC; ++half) {
	    	    			ParseDAQHeader(m_words[line.header_vldb_id][asic][half], 
                                           hd, bx, ec, ob, ham, tr);

                            HeaderData daq_header(line.header_vldb_id, 
                                                  asic, half, hd, bx, ec, ob, ham, tr);

                            CheckDAQHeader(daq_header);
                            buffer_daqh.push_back(daq_header);
	    	    		}
	    	    	}
                }

                // Here we handle all the regular data lines that have channel information
                // We skip four specific channels: DAQ header, common mode, calib. and CRC
	    	    if (not (m_vldb_line_counter[line.header_vldb_id] == 0 ||
	    	    		 m_vldb_line_counter[line.header_vldb_id] == 1 ||
	    	    		 m_vldb_line_counter[line.header_vldb_id] == 20 ||
	    	    		 m_vldb_line_counter[line.header_vldb_id] == 39 )) {

	    	    	m_current_channel[line.header_vldb_id] = m_vldb_line_counter_to_channel_index[line.header_vldb_id];
	    	    	m_vldb_line_counter_to_channel_index[line.header_vldb_id] += 1;
	    	    	m_words[line.header_vldb_id] = { { line.data_word0, line.data_word1 },
	    	    				                     { line.data_word2, line.data_word3 } };
                            
                    // Check if the found VLDB link ID is legal
                    // VLDB link IDs are zero-indexed
                    if (line.header_vldb_id >= g_NUM_VLDB) {
                        spdlog::error("Got data line for VLDB link {}, but we only have {} links!", 
                                         line.header_vldb_id, g_NUM_VLDB);
                    }

                    // Check if the current channel we've counted to is legal
                    // Channel indices are zero-indexed
                    if (m_current_channel[line.header_vldb_id] >= g_NUM_CHANNELS_PER_HALF) {
                        spdlog::error("Channel number counter for VLDB link {} in current DAQ frame is {} (must be < {})",
                                         line.header_vldb_id,
                                         m_current_channel[line.header_vldb_id],
                                         g_NUM_CHANNELS_PER_HALF);

                    }

                    // Check if the current machine gun number is legal
                    // Machine gun numbers are one-indexed
                    if (m_current_machinegun[line.header_vldb_id] > g_NUM_MACHINE_GUN_TRIGGERS) {
                        spdlog::error("Machine gun counter for VLDB link {} in current event is {} (must be < {})",
                                         line.header_vldb_id,
                                         m_current_machinegun[line.header_vldb_id],
                                         g_NUM_MACHINE_GUN_TRIGGERS+1);
                    }

	    	    	bool tc, tp;
	    	    	unsigned long int adc, tot, toa;
	    	    	for (unsigned int asic = 0; asic < g_NUM_ASIC_PER_VLDB; ++asic) {
	    	    		for (unsigned int half = 0; half < g_NUM_HALVES_PER_ASIC; ++half) {
	    	    			ParseWord(m_words[line.header_vldb_id][asic][half], tc, tp, adc, tot, toa);

	    	    			ChannelData tmp_adc(line.header_vldb_id, 
                                                asic, half, m_current_channel[line.header_vldb_id], 
                                                m_current_machinegun[line.header_vldb_id], adc);

	    	    			ChannelData tmp_tot(line.header_vldb_id, 
                                                asic, half, m_current_channel[line.header_vldb_id], 
                                                m_current_machinegun[line.header_vldb_id], tot);

	    	    			ChannelData tmp_toa(line.header_vldb_id, 
                                                asic, half, m_current_channel[line.header_vldb_id], 
                                                m_current_machinegun[line.header_vldb_id], toa);

	    	    			buffer_adc.push_back(tmp_adc);
	    	    			buffer_tot.push_back(tmp_tot);
	    	    			buffer_toa.push_back(tmp_toa);
                            
                            m_running_event_adc[line.header_vldb_id] += adc;
	    	    		}
	    	    	}
	    	    }

	    	    m_vldb_line_counter[line.header_vldb_id] += 1;
            }
	   
            // For the sake of log formatting, debug output goes here    
	    	spdlog::get("Integrity")->debug("[DATA] {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X}",
                          line.header_type,
                          line.header_vldb_id,
                          line.bx_cnt,
                          line.ob_cnt,
                          line.data_word0,
                          line.data_word1,
                          line.data_word2,
                          line.data_word3,
                          line.data_word4,
                          line.data_word5);

            if (m_vldb_line_counter[line.header_vldb_id] == g_VLDB_LINES_PER_EVENT) {
                OnDAQFrameEnd(line);
                if (m_current_machinegun[line.header_vldb_id] == g_NUM_MACHINE_GUN_TRIGGERS) {
                    OnEventEnd(line);
                }
            }
        },
        
        // Function to run when we get a trigger packet
        [&](const bp::TrgLine& line, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
            spdlog::get("Integrity")->debug("[TRIG] {:08X} {:08X} {:08X} {:08X} {:08X}",
                          line.header_type,
                          line.bx_cnt,
                          line.ob_cnt,
                          line.reserved0,
                          line.reserved1);

			++trig_lines_read;

            // New trigger arrived while previous one was still active - this is an exception
            // Reset internal state and carry on with the new event
            //if (m_has_active_trigger) {
            for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
                if (m_has_active_trigger[vldb]) {
                    spdlog::get("Integrity")->warn("Got new trigger line, but VLDB link {} was not done yet (trigger #{})", vldb, trig_lines_read);
                    ++num_bad_triggers;
                    OnEventException();
                    OnEventStart();
                    break;
                }
            }

            // Otherwise, new trigger arrived while we had no active trigger
            // This is normal behaviour and lets us know that an event is incoming
            //else {
                ++num_good_triggers;
                OnEventStart();
            //}
        }
    );

}

void DataReader::OnEventStart() {
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        m_running_event_adc[vldb] = 0;
        m_current_machinegun[vldb] = 0;
        m_is_event_tainted[vldb] = false;
        m_has_active_trigger[vldb] = true;
    }


    spdlog::get("Integrity")->debug("╒══════════════════════════════════════╪ START OF EVENT ╪══════════════════════════════════════╕");
    //spdlog::get("Integrity")->debug("╽ ┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉  EVENT #{:05d}  ┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉┉ ╽", num_complete_events[line.header_vldb_id] + num_incomplete_events[line.header_vldb_id]);
}

void DataReader::OnEventEnd(const bp::DataLine& line) {
    buffer_machineguns.push_back(m_current_machinegun[line.header_vldb_id]);
    event_buffer.push_back(EventData(m_running_event_adc[line.header_vldb_id], m_current_machinegun[line.header_vldb_id]));

    if (m_current_machinegun[line.header_vldb_id] == g_NUM_MACHINE_GUN_TRIGGERS) { 
        ++num_complete_events[line.header_vldb_id]; 
    } else {
        spdlog::get("Integrity")->warn("VLDB Link {} had fewer ({}) than expected ({}) machine gun triggers for event {} (event ended normally)", line.header_vldb_id, m_current_machinegun[line.header_vldb_id], g_NUM_MACHINE_GUN_TRIGGERS, (num_complete_events[line.header_vldb_id] + num_incomplete_events[line.header_vldb_id]));
        ++num_incomplete_events[line.header_vldb_id];
    }

    if (m_is_event_tainted[line.header_vldb_id]) {
        ++num_bad_events[line.header_vldb_id];

        if (not m_has_shown_taint_warning) {
            m_has_shown_taint_warning = true;
            spdlog::warn("A data integrity error has been found in the current run - this error (and all future errors) can be found in ./data-integrity.log. This message will not be shown again for the current run.");
        }
    }

    m_has_active_trigger[line.header_vldb_id] = false;
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        if (m_has_active_trigger[vldb]) {
            spdlog::get("Integrity")->debug("Event not yet done for VLDB link {}", vldb);
            return;
        }
    }

        spdlog::get("Integrity")->debug("╘══════════════════════════════════════╧  END OF EVENT  ╧══════════════════════════════════════╛");
}

void DataReader::OnEventException() {
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        buffer_machineguns.push_back(m_current_machinegun[vldb]);
        event_buffer.push_back(EventData(m_running_event_adc[vldb], m_current_machinegun[vldb]));

        ++num_bad_events[vldb];
        if ((m_current_machinegun[vldb] == g_NUM_MACHINE_GUN_TRIGGERS) && (not m_is_reading_daq_frame[vldb])) {
            ++num_complete_events[vldb];
        } else {
            spdlog::get("Integrity")->warn("VLDB Link {} had fewer ({}) than expected ({}) machine gun triggers for event {} (event ended by exception)", vldb, m_current_machinegun[vldb], g_NUM_MACHINE_GUN_TRIGGERS, (num_complete_events[vldb] + num_incomplete_events[vldb]));
            ++num_incomplete_events[vldb];
        }

        m_is_reading_daq_frame[vldb] = 0;
        m_vldb_line_counter[vldb] = 0;
        m_vldb_line_counter_to_channel_index[vldb] = 0;
        m_current_channel[vldb] = -1;
    }
}

void DataReader::OnDAQFrameStart(const bp::DataLine& line) {

    frame_start_time[line.header_vldb_id] = line.bx_cnt + (line.ob_cnt << 12);
    m_current_machinegun[line.header_vldb_id] += 1;
    m_is_reading_daq_frame[line.header_vldb_id] = true;
    
    spdlog::get("Integrity")->debug("┍━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━─ START OF DAQ FRAME ─━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
    spdlog::get("Integrity")->debug("╽ ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈      VLDB Link #{}     ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ ╽", line.header_vldb_id);
    spdlog::get("Integrity")->debug("╽ ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈  MACHINE GUN TRG. #{:02d}  ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ ╽", m_current_machinegun[line.header_vldb_id]);
}

void DataReader::OnDAQFrameEnd(const bp::DataLine& line) {
    m_vldb_line_counter[line.header_vldb_id] = 0;
    m_vldb_line_counter_to_channel_index[line.header_vldb_id] = 0;

    m_current_channel[line.header_vldb_id] = -1;
    m_is_reading_daq_frame[line.header_vldb_id] = false;

    spdlog::get("Integrity")->debug("╿ ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈  MACHINE GUN TRG. #{:02d}  ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ ╿", m_current_machinegun[line.header_vldb_id]);
    spdlog::get("Integrity")->debug("╿ ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈      VLDB Link #{}     ┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈┈ ╿", line.header_vldb_id);
    spdlog::get("Integrity")->debug("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━─ END OF DAQ FRAME ─━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
}

void DataReader::DoTailing() {
    spdlog::debug("Tailing started");
    bp::TailOptions opts;
    opts.poll_ms = 1000;
    opts.read_chunk = 1u << 20;
    opts.inactivity_timeout_ms = 0;
    std::vector<std::byte> stash;
    bp::tail_growing_file(m_target, std::ref(m_interrupt_tailing), opts, 
            [&](std::span<const std::byte> chunk) {
                bytes_read += chunk.size();
                if (!stash.empty()) {
                    const std::size_t need = bp::ByteCursor::kLineSize - stash.size();
                    if (chunk.size() >= need) {
                        std::vector<std::byte> one(stash.begin(), stash.end());
                        one.insert(one.end(), chunk.begin(), chunk.begin() + need);
                        mp_parser->feed(one);
                        chunk = chunk.subspan(need);
                        stash.clear();
                    } else {
                        stash.insert(stash.end(), chunk.begin(), chunk.end());
                        return;
                    }
                }

                const std::size_t remainder = chunk.size() % bp::ByteCursor::kLineSize;
                const auto main_part = chunk.first(chunk.size() - remainder);
                if (!main_part.empty()) {
                    mp_parser->feed(main_part);
                }

                if (remainder) {
                    stash.assign(chunk.end() - remainder, chunk.end());
                }
            });
}

void DataReader::Start() {
	if (not std::filesystem::exists(m_target)) {
		spdlog::error("Target file {} does not exist or can't be opened", m_target);
	} else {
		spdlog::info("Starting DataReader");
        if (not m_interrupt_tailing) {
            spdlog::warn("DataReader appears to already be running "
                         "- if this is not the case, restart the monitor. "
                         "Ignoring request.");
        } else {
            m_interrupt_tailing = false;
            std::thread tail_thread(&DataReader::DoTailing, this);
            tail_thread.detach();
        }
	}
}
							

void DataReader::Stop() {
	m_interrupt_tailing = true;
	spdlog::info("Stopping DataReader");
}

void DataReader::ParseWord(	unsigned long int& word, bool& tc, bool& tp, 
							unsigned long int& adc, unsigned long int& tot, 
							unsigned long int& toa) {

	tc	= (word >> 31) & 0x1;
	tp	= (word >> 30) & 0x1;
	adc	= (word >> 20) & 0x3ff;
	tot	= (word >> 10) & 0x3ff;
	toa = (word      ) & 0x3ff;
}

void DataReader::ParseDAQHeader(unsigned long int& word, unsigned int& hd,
                                unsigned int& bx, unsigned int& ec,
                                unsigned int& ob, unsigned int& ham,
                                unsigned int& tr) {

    tr  = (word      ) & 0xf;
    ham = (word >> 4 ) & 0x7;
    ob  = (word >> 7 ) & 0x7;
    ec  = (word >> 10) & 0x3f;
    bx  = (word >> 16) & 0xfff;
    hd  = (word >> 28) & 0xf;
}

void DataReader::CheckDAQHeader(HeaderData header) {
    bool is_good = true;
    if (not ((header.hd == 0xf) && (header.tr == 0x5))) {
        is_good = false;
        spdlog::get("Integrity")->error("VLDB Link {}, ASIC {}.{}: DAQH header and/or tailer pattern mismatch in event {}, mg {} (got: {:01X}{:01X}, expected F5", header.vldb_id, header.asic_id, header.half, num_complete_events[header.vldb_id] + num_incomplete_events[header.vldb_id], m_current_machinegun[header.vldb_id], header.hd, header.tr);
        //spdlog::get("Integrity")->error("DAQH header and/or tailer pattern mismatch! Got {:01X}{:01X}, expected F5 (event {}, VLDB link {}, ASIC {}.{})", header.hd, header.tr, num_complete_events[header.vldb_id] + num_incomplete_events[header.vldb_id] + 1, header.vldb_id, header.asic_id, header.half);
    }

    if (not (header.ham == 0)) {
        is_good = false;
        spdlog::get("Integrity")->error("VLDB Link {}, ASIC {}.{}: Hamming decode error bits are set in event {}, mg {} (bits: {:03b})", header.vldb_id, header.asic_id, header.half, num_complete_events[header.vldb_id] + num_incomplete_events[header.vldb_id], m_current_machinegun[header.vldb_id], header.ham);
        //spdlog::get("Integrity")->error("Hamming decode error bits are set for event {} on VLDB link {}, ASIC {}.{}! (bits: {:03b})", num_complete_events[header.vldb_id] + num_incomplete_events[header.vldb_id] + 1, header.vldb_id, header.asic_id, header.half, header.ham);
    }

    if (not is_good) {
        ++integrity_check_fails;
        ++error_counts[header.vldb_id][header.asic_id];
        m_is_event_tainted[header.vldb_id] = true;
    }
}

void DataReader::SetTarget(std::string path) {
	spdlog::info("Target file changed to {}", path);
    Reset();
	m_target = path;
}

std::string DataReader::GetTarget() {
	return m_target;
}

bool DataReader::IsRunning() {
	return (not m_interrupt_tailing);
}

void DataReader::ClearBuffers() {
    spdlog::debug("Clearing data buffers");
    buffer_bx_counters.clear();
    buffer_machineguns.clear();
    buffer_adc.clear();
    buffer_tot.clear();
    buffer_toa.clear();
    event_buffer.clear();
    buffer_daqh.clear();
}

void DataReader::Reset() {
    spdlog::debug("Resetting DataReader");

    num_packets             = 0;
    num_heartbeat_packets   = 0;
    num_sync_packets        = 0;
    bytes_read              = 0;
    data_lines_read         = 0;
    trig_lines_read         = 0;
    l0_lines_read           = 0;
    l1_lines_read           = 0;
    
    num_good_triggers       = 0;
    num_bad_triggers        = 0;

    std::memset(error_counts, 0, sizeof error_counts);
    integrity_check_fails   = 0;
    
    buffer_bx_counters.clear();
    buffer_machineguns.clear();
    buffer_adc        .clear();
    buffer_tot        .clear();
    buffer_toa        .clear();
    event_buffer      .clear();
    buffer_daqh       .clear();
    
    m_interrupt_tailing       = true;
    m_has_shown_taint_warning = false;

    for (int i = 0; i < g_NUM_VLDB; ++i) {
        m_vldb_line_counter[i]                  = 0;
        m_vldb_line_counter_to_channel_index[i] = 0;
        m_vldb_bx_counter[i]                    = 0;

        num_complete_events[i]      = 0;
        num_incomplete_events[i]    = 0;
        num_bad_events[i]           = 0;

        frame_start_time[i] = 0;

        m_current_channel[i]           = 0;
        m_current_machinegun[i]        = 0;
        m_running_event_adc[i]         = 0;
        m_is_event_tainted[i]          = false;
        m_is_reading_daq_frame[i]      = false;
        m_has_active_trigger[i]        = false;
    }
}
