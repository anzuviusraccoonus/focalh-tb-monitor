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

        unsigned int num_complete_events;
        unsigned int num_incomplete_events;
        unsigned int num_good_triggers;
        unsigned int num_bad_triggers;
        
        long int frame_start_time;

        std::vector<int> buffer_bx_counters = {};
        std::vector<int> buffer_machineguns = {};
        std::vector<ChannelData> buffer_adc = {};
        std::vector<ChannelData> buffer_tot = {};
        std::vector<ChannelData> buffer_toa = {};
        std::vector<EventData> event_buffer = {};

    private:
        DataReader();

        void OnEventStart();
        void OnEventEnd();
        void OnDAQFrameStart(const bp::DataLine& line);
        void OnDAQFrameEnd();

        static DataReader* mp_instance;
        bp::StreamParser* mp_parser;

		int m_current_channel;
        int m_current_machinegun;
        int m_running_event_adc;
        int m_vldb_bx_counter[g_NUM_VLDB];
        int m_vldb_line_counter[g_NUM_VLDB];
        int m_vldb_line_counter_to_channel_index[g_NUM_VLDB];
		bool m_is_reading_daq_frame;
		bool m_has_active_trigger;
		bool m_interrupt_tailing;
		std::string m_target;
		std::vector<std::vector<unsigned long int>> m_words{{NULL, NULL}, {NULL, NULL}};
};

#endif
