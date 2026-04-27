#ifndef __MONITOR_INTEGRITYPAGE_H__
#define __MONITOR_INTEGRITYPAGE_H__

#include "Page.h"

class IntegrityPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();
};

#endif
