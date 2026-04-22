#ifndef __MONITOR_CHANNELSPAGE_H__
#define __MONITOR_CHANNELSPAGE_H__

#include "Page.h"

class ChannelsPage : public Page {
	using Page::Page;

	public:
		void Initialize();
		void Update();
		void Clear();
		void Reset();

		void SetBoardIDs(unsigned int vldb_id, unsigned int asic_id, unsigned int half);

	private:
		unsigned long int m_buffer_idx;
		unsigned int m_vldb;
		unsigned int m_asic;
		unsigned int m_half;
};

#endif
