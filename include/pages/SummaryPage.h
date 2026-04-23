#ifndef __MONITOR_SUMMARYPAGE_H__
#define __MONITOR_SUMMARYPAGE_H__

#include "Page.h"

class SummaryPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();

	private:
        unsigned int m_buffer_idx;
};

#endif
