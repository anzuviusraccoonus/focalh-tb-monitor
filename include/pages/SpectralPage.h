#ifndef __MONITOR_SPECTRALPAGE_H__
#define __MONITOR_SPECTRALPAGE_H__

#include "Page.h"

class SpectralPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();

	private:
		unsigned long int m_buffer_idx;
};

#endif
