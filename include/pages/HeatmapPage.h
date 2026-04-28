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
        int m_num_events;
        float m_temp_heatmap_data[g_NUM_MACHINE_GUN_TRIGGERS][g_HEATMAP_NUM_ROWS][g_HEATMAP_NUM_COLS];
};

#endif
