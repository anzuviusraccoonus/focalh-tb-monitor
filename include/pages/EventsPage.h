#ifndef __MONITOR_EVENTSPAGE_H__
#define __MONITOR_EVENTSPAGE_H__

#include "Page.h"

class EventsPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();

	private:
        int m_num_complete_events_last[g_NUM_VLDB];
        int m_num_incomplete_events_last[g_NUM_VLDB];
        int m_num_bad_events_last[g_NUM_VLDB];
};

#endif
