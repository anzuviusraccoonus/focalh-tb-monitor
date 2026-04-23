#ifndef __MONITOR_DATASTRUCTS_H__
#define __MONITOR_DATASTRUCTS_H__

struct ChannelData {
	unsigned int vldb_id = 0;
	unsigned int asic_id = 0;
	unsigned int half = 0;
	unsigned int chn = 0;
	unsigned int mg = 0;
	unsigned int value = 0;
};

struct EventData {
    int adc = 0;
    int num_machineguns = 0;
};

#endif
