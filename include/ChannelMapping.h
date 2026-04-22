#ifndef __MONITOR_CHANNELMAPPING_H__
#define __MONITOR_CHANNELMAPPING_H__

#include <string>
#include <fstream>
#include <TObject.h>

#include "globals.h"

class ChannelMapping : public TObject {
	public:
		static ChannelMapping* GetInstance() {
			if (mp_instance == nullptr) {
				mp_instance = new ChannelMapping();
			}

			return mp_instance;
		}

        void LoadMapping(std::string path);

        void PrintMapping();

        std::pair<int, int> GetRowCol(int vldb, int asic, int half, int chn);

	private:
		ChannelMapping();

		static ChannelMapping* mp_instance;
        std::pair<int, int> m_mapping[g_NUM_VLDB][g_NUM_ASIC_PER_VLDB][g_NUM_HALVES_PER_ASIC][g_NUM_CHANNELS_PER_HALF];
};

#endif
