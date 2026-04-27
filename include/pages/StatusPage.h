#ifndef __MONITOR_STATUSPAGE_H__
#define __MONITOR_STATUSPAGE_H__

#include <TText.h>
#include "Page.h"

class StatusPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();
		
	private:
		TText* mp_file;
		TText* mp_reader_running;
		TText* mp_data_read;
		TText* mp_lines_read;
		TText* mp_trig_lines_read;
		TText* mp_data_lines_read;
		TText* mp_num_heartbeat_packets;
		TText* mp_num_sync_packets;
		TText* mp_l0_lines_read;
		TText* mp_l1_lines_read;
        TText* mp_num_errors_desc;
        TText* mp_num_errors;

        int m_num_trig_lines_read;
        int m_num_data_lines_read;
        TText* mp_last_trig_time;
        TText* mp_last_data_time;
        TText* mp_last_server_update_time;
};

#endif
