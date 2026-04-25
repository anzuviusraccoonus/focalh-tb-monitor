#ifndef __MONITOR_HEATMAPPAGE_H__
#define __MONITOR_HEATMAPPAGE_H__

#include "Page.h"

class HeatmapPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();

	private:
		unsigned long int m_buffer_idx;
        unsigned int m_num_events;
};

#endif
