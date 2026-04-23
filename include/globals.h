#ifndef __MONITOR_GLOBALS_H__
#define __MONITOR_GLOBALS_H__

#include <mutex>

constexpr int g_NUM_VLDB = 2;
constexpr int g_NUM_ASIC_PER_VLDB = 2;
constexpr int g_NUM_HALVES_PER_ASIC = 2;
constexpr int g_NUM_CHANNELS_PER_HALF = 36;
constexpr int g_NUM_MACHINE_GUN_TRIGGERS = 16;
constexpr int g_VLDB_LINES_PER_EVENT = 40;
constexpr int g_HEATMAP_NUM_ROWS = 16;
constexpr int g_HEATMAP_NUM_COLS = 12;

extern std::mutex g_mutex;
extern double g_timegraphs_window_seconds;
extern unsigned int g_timegraphs_num_points;
extern unsigned int g_server_port;

#endif
