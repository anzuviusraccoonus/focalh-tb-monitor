#ifndef __MONITOR_TSPAGE1_H__
#define __MONITOR_TSPAGE1_H__

#include "Page.h"

class TSPage1 : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();

	private:
        unsigned int m_buffer_idx;
        int m_num_complete_events;
        int m_num_incomplete_events;
        int m_num_good_triggers;
        int m_num_bad_triggers;
};

#endif
