#ifndef __MONITOR_DATAREADER_H__
#define __MONITOR_DATAREADER_H__

#include "../external/binparse/include/binparse/parser.hpp"
#include "../external/binparse/include/binparse/bytecursor.hpp"
#include "../external/binparse/include/binparse/tail.hpp"
#include "DataStructs.h"
#include "globals.h"

class DataReader {
    public:
        static DataReader* GetInstance() {
            if (mp_instance == nullptr) {
                mp_instance = new DataReader();
            }

            return mp_instance;
        }

        void Start();
		void Stop();
        void Reset();
		void SetTarget(std::string path);
		void DoTailing();
		std::string GetTarget();
		bool IsRunning();
		void ParseWord(	unsigned long int& word, bool& tc, bool& tp, 
						unsigned long int& adc, unsigned long int& tot, 
						unsigned long int& toa);

        void ParseDAQHeader(unsigned long int& word, unsigned int& hd,
                            unsigned int& bx, unsigned int& ec,
                            unsigned int& ob, unsigned int& ham,
                            unsigned int& tr);

        void CheckDAQHeader(HeaderData header);
        void ClearBuffers();


        // Public data members, to be accessed by graphs / pages
        //
        // Pages access is only when PageManager::UpdatePages() is called,
        // which uses the global mutex lock, which the data reader also does
        // whenever it processes a line, so these should be safe from race conditions

        unsigned int num_packets;
        unsigned int num_heartbeat_packets;
        unsigned int num_sync_packets;
        unsigned int bytes_read;
        unsigned int data_lines_read;
        unsigned int trig_lines_read;
        unsigned int l0_lines_read;
        unsigned int l1_lines_read;

        unsigned int num_complete_events[g_NUM_VLDB];
        unsigned int num_incomplete_events[g_NUM_VLDB];
        unsigned int num_bad_events[g_NUM_VLDB];
        unsigned int num_good_triggers;
        unsigned int num_bad_triggers;

        unsigned int error_counts[g_NUM_VLDB][g_NUM_ASIC_PER_VLDB];
        unsigned int integrity_check_fails;
        
        long int frame_start_time[g_NUM_VLDB];

        std::vector<int> buffer_bx_counters = {};
        std::vector<int> buffer_machineguns = {};
        std::vector<ChannelData> buffer_adc = {};
        std::vector<ChannelData> buffer_tot = {};
        std::vector<ChannelData> buffer_toa = {};
        std::vector<EventData> event_buffer = {};
        std::vector<HeaderData> buffer_daqh = {};

    private:
        DataReader();

        void OnEventStart();
        void OnEventEnd(const bp::DataLine& line);
        void OnEventException();
        void OnDAQFrameStart(const bp::DataLine& line);
        void OnDAQFrameEnd(const bp::DataLine& line);

        static DataReader* mp_instance;
        bp::StreamParser* mp_parser;

		int m_current_channel[g_NUM_VLDB];
        int m_current_machinegun[g_NUM_VLDB];
        int m_running_event_adc[g_NUM_VLDB];
        int m_vldb_bx_counter[g_NUM_VLDB];
        int m_vldb_line_counter[g_NUM_VLDB];
        int m_vldb_line_counter_to_channel_index[g_NUM_VLDB];
        bool m_is_event_tainted[g_NUM_VLDB];
		bool m_is_reading_daq_frame[g_NUM_VLDB];
		bool m_has_active_trigger;
		bool m_interrupt_tailing;
		std::string m_target;
		std::vector<std::vector<unsigned long int>> m_words[g_NUM_VLDB];//{{NULL, NULL}, {NULL, NULL}};
};

#endif
