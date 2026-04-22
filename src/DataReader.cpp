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
            spdlog::debug("[HRT.]");
			++num_heartbeat_packets;
        },
        
        // Function to run when we get a sync packet(?)
        [&](std::span<const std::byte>) {
            std::scoped_lock lock(g_mutex);
            spdlog::debug("[SYNC]");
			++num_sync_packets;
        },
        
        // Function to run when we get an RDH L0 packet
        [&](const bp::RDH_L0& rdh, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
            spdlog::debug("[RDH0] {:02X} {:02X} {:04X} {:02X} {:02X} {:04X} {:04X} {:04X} {:02X} {:02X} {:04X} {:02X} {:04X} {:08X} {:08X} {:02X} {:08X} {:08X}",
                          rdh.header_version,
                          rdh.header_size,
                          rdh.fee_id,
                          rdh.priority_bit,
                          rdh.system_id,
                          rdh.reserved0,
                          rdh.offset_new_packet,
                          rdh.memory_size,
                          rdh.link_id,
                          rdh.packet_counter,
                          rdh.cru_id,
                          rdh.dw,
                          rdh.bc,
                          rdh.reserved1,
                          rdh.orbit,
                          rdh.data_format,
                          rdh.reserved2,
                          rdh.reserved3 );
			++l0_lines_read;
        },
        
        // Function to run when we get an RDH L1 packet
        [&](const bp::RDH_L1& rdh, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
            spdlog::debug("[RDH1] {:08X} {:04X} {:02X} {:02X} {:08X} {:08X} {:08X} {:04X} {:04X} {:08X} {:08X}",
                          rdh.trg_type, 
                          rdh.hb_packet_counter,
                          rdh.stop_bit,
                          rdh.reserved0,
                          rdh.reserved1,
                          rdh.reserved2,
                          rdh.detector_field,
                          rdh.par_bit,
                          rdh.reserved3,
                          rdh.reserved4,
                          rdh.reserved5 );
			++l1_lines_read;
        },
        
        // Function to run when we get a data packet
        [&](const bp::DataLine& line, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
			++data_lines_read;

            // Don't read any data lines unless we got a trigger line first
            if (not m_has_active_trigger) { return; }

            // This block determines when we've found the start of an event frame,
            // following a valid trigger line
	    	if (not m_is_reading_event_frame) {
                auto magic = line.data_word0 & 0xF000000F;
                if (magic == 0xF0000005 || magic == 0xF0000002) {
	    			m_current_machinegun += 1;
	    			spdlog::debug("╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤ START OF EVENT FRAME ╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤╤");
	    			spdlog::debug("╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽ MACHINE GUN TRG. #{:02d} ╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽╽", m_current_machinegun);

	    		    frame_start_time = line.bx_cnt + (line.ob_cnt << 12);	
                    m_is_reading_event_frame = true;
	    			for (int i = 0; i < g_NUM_VLDB; ++i) {
	    				m_vldb_line_counter[i] = 0;
                        m_vldb_line_counter_to_channel_index[i] = 0;
                        m_vldb_bx_counter[i] = line.bx_cnt - 1;
	    			}
	    		}
	    	}

            // If not currently reading event frame, i.e. the above block
            // didn't find the start of the event frame, we skip this
            if (m_is_reading_event_frame) {
                if (not (line.bx_cnt == (m_vldb_bx_counter[line.header_vldb_id] + 1))) {
                    if ((line.bx_cnt == 0) & (m_vldb_bx_counter[line.header_vldb_id] == 3563)) {
                        m_vldb_bx_counter[line.header_vldb_id] = -1;
                    }
                }

                buffer_bx_counters.push_back(line.bx_cnt - m_vldb_bx_counter[line.header_vldb_id]);
                m_vldb_bx_counter[line.header_vldb_id] += 1;

	    	    if (not (m_vldb_line_counter[line.header_vldb_id] == 0 ||
	    	    		 m_vldb_line_counter[line.header_vldb_id] == 1 ||
	    	    		 m_vldb_line_counter[line.header_vldb_id] == 20 ||
	    	    		 m_vldb_line_counter[line.header_vldb_id] == 39 )) {

	    	    	m_current_channel = m_vldb_line_counter_to_channel_index[line.header_vldb_id];
	    	    	m_vldb_line_counter_to_channel_index[line.header_vldb_id] += 1;
	    	    	m_words = {	{ line.data_word0, line.data_word1 },
	    	    				{ line.data_word2, line.data_word3 } 	};

	    	    	bool tc, tp;
	    	    	unsigned long int adc, tot, toa;
	    	    	for (unsigned int asic = 0; asic < g_NUM_ASIC_PER_VLDB; ++asic) {
	    	    		for (unsigned int half = 0; half < g_NUM_HALVES_PER_ASIC; ++half) {
	    	    			ParseWord(m_words[asic][half], tc, tp, adc, tot, toa);
                            if (line.header_vldb_id > g_NUM_VLDB-1) {
                                spdlog::critical("Got data line for VLDB link {}, but we only have {} links!", line.header_vldb_id, g_NUM_VLDB);
                            }

	    	    			ChannelData tmp_adc(line.header_vldb_id, 
                                                asic, half, m_current_channel, 
                                                m_current_machinegun, adc);
	    	    			buffer_adc.push_back(tmp_adc);

	    	    			ChannelData tmp_tot(line.header_vldb_id, 
                                                asic, half, m_current_channel, 
                                                m_current_machinegun, tot);
	    	    			buffer_tot.push_back(tmp_tot);

	    	    			ChannelData tmp_toa(line.header_vldb_id, 
                                                asic, half, m_current_channel, 
                                                m_current_machinegun, toa);
	    	    			buffer_toa.push_back(tmp_toa);
	    	    		}
	    	    	}
	    	    }

	    	    m_vldb_line_counter[line.header_vldb_id] += 1;
            }
	    	
	    	// For the sake of log formatting, debug output goes here
	    	spdlog::debug(	"[DATA] {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X} {:08X}",
	    					line.header_type,
	    					line.header_vldb_id,
	    					line.bx_cnt,
	    					line.ob_cnt,
	    					line.data_word0,
	    					line.data_word1,
	    					line.data_word2,
	    					line.data_word3,
	    					line.data_word4,
	    					line.data_word5	);

            // This block determines whether we've gotten to the end of an event frame
	    	if (m_vldb_line_counter[g_NUM_VLDB-1] == g_VLDB_LINES_PER_EVENT) {
	    		spdlog::debug("╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿ MACHINE GUN TRG. #{:02d} ╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿╿", m_current_machinegun);
	    		spdlog::debug("╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧  END OF EVENT FRAME  ╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧╧");
	    		m_is_reading_event_frame = false;
                if (m_current_machinegun == g_NUM_MACHINE_GUN_TRIGGERS-1) {
                    ++num_complete_events;
                    m_has_active_trigger = false;
                    buffer_machineguns.push_back(m_current_machinegun+1);
                }

	    		for (int i = 0; i < g_NUM_VLDB; ++i) {
	    			m_vldb_line_counter[i] = 0;
	    			m_vldb_line_counter_to_channel_index[i] = 0;
	    		}
	    	}

        },
        
        // Function to run when we get a trigger packet
        [&](const bp::TrgLine& line, std::span<const std::byte> raw) {
            std::scoped_lock lock(g_mutex);
            spdlog::debug("[TRIG] {:08X} {:08X} {:08X} {:08X} {:08X}",
                          line.header_type,
                          line.bx_cnt,
                          line.ob_cnt,
                          line.reserved0,
                          line.reserved1);

			++trig_lines_read;

            // New trigger arrived while previous one was still active - this is an exception
            if (m_has_active_trigger) {

                // The trigger arrived *during* an event frame
                // Ignore it and log a warning for a lost trigger
                if (m_is_reading_event_frame) {
                    spdlog::warn("Got a trigger line during an active event frame");
                    ++num_bad_triggers;
                }

                // The trigger arrived *between* event frames, i.e. before all machine guns were read
                // This counts as a partial / incomplete event
                else {
                    spdlog::warn("Event had fewer than expected machine gun triggers");
                    buffer_machineguns.push_back(m_current_machinegun+1);
                    m_current_machinegun = -1;
                    ++num_good_triggers;
                    m_is_reading_event_frame = false;
                    for (int i = 0; i < g_NUM_VLDB; ++i) {
                        m_vldb_line_counter[i] = 0;
                        m_vldb_line_counter_to_channel_index[i] = 0;
                    }
                }
            }

            // Otherwise, new trigger arrived while we had no active trigger
            // This is normal behaviour and lets us know that an event is incoming
            else {

                // The trigger arrived at the appropriate time
                if (not m_is_reading_event_frame) {
                    spdlog::debug("     ↳ Got a new trigger and everything seems good");
                    m_current_machinegun = -1;
                    ++num_good_triggers;
                    m_has_active_trigger = true;
                }

                // The trigger arrived while we were still reading an event frame,
                // but with no active trigger - this should never happen
                else {
                    spdlog::critical("Got new trigger while an event frame was active but no trigger was active! This should not be possible!!");
                }
            }
        }
    );

}

DataReader::~DataReader() {
}

void DataReader::DoTailing() {
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
		spdlog::info("DataReader started");
		m_interrupt_tailing = false;
		std::thread tail_thread(&DataReader::DoTailing, this);
		tail_thread.detach();
	}
}
							

void DataReader::Stop() {
	m_interrupt_tailing = true;
	spdlog::info("DataReader stopped");
}

void DataReader::ParseWord(	unsigned long int& word, bool& tc, bool& tp, 
							unsigned long int& adc, unsigned long int& tot, 
							unsigned long int& toa) {

	tc	=	(word >> 31) & 0x1;
	tp	=	(word >> 30) & 0x1;
	adc	=	(word >> 20) & 0x3ff;
	tot	=	(word >> 10) & 0x3ff;
	toa	=	(word      ) & 0x3ff;
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

void DataReader::Reset() {
    num_packets             = 0;
    num_heartbeat_packets   = 0;
    num_sync_packets        = 0;
    bytes_read              = 0;
    data_lines_read         = 0;
    trig_lines_read         = 0;
    l0_lines_read           = 0;
    l1_lines_read           = 0;
    
    num_complete_events     = 0;
    num_incomplete_events   = 0;
    num_good_triggers       = 0;
    num_bad_triggers        = 0;
        
    frame_start_time        = 0;
    
    buffer_bx_counters.clear();
    buffer_machineguns.clear();
    buffer_adc        .clear();
    buffer_tot        .clear();
    buffer_toa        .clear();

    m_current_channel           = 0;
    m_current_machinegun        = 0;
    m_is_reading_event_frame    = false;
    m_has_active_trigger        = false;
    m_interrupt_tailing         = true;

    for (int i = 0; i < g_NUM_VLDB; ++i) {
        m_vldb_line_counter[i] = 0;
        m_vldb_line_counter_to_channel_index[i] = 0;
        m_vldb_bx_counter[i] = 0;
    }
}
