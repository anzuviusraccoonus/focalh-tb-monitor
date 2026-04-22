#ifndef __MONITOR_TSPAGE2_H__
#define __MONITOR_TSPAGE2_H__

#include "Page.h"

class TSPage2 : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();
};

#endif
