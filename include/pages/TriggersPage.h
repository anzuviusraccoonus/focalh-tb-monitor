#ifndef __MONITOR_TRIGGERSPAGE_H__
#define __MONITOR_TRIGGERSPAGE_H__

#include "Page.h"

class TriggersPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();

	private:
        int m_num_good_triggers_last;
        int m_num_bad_triggers_last;
};

#endif
