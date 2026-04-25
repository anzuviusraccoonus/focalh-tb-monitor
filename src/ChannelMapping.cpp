#include <filesystem>
#include "spdlog/spdlog.h"
#include "ChannelMapping.h"
#include "Server.h"

ChannelMapping* ChannelMapping::mp_instance = nullptr;

ChannelMapping::ChannelMapping() {
    spdlog::info("Initializing ChannelMapping");
    InitMapping();

    if (std::filesystem::exists("./channelmapping")) {
        spdlog::info("Loading default channel mapping");
        LoadMapping("./channelmapping");
    } else {
        spdlog::info("Default channel mapping not found; heatmaps will not be available until one is loaded");
    }
}

void ChannelMapping::InitMapping() {
    spdlog::debug("Init channel mapping");
    for (int i = 0; i < g_NUM_VLDB; ++i) {
        for (int j = 0; j < g_NUM_ASIC_PER_VLDB; ++j) {
            for (int k = 0; k < g_NUM_HALVES_PER_ASIC; ++k) {
                for (int chn = 0; chn < g_NUM_CHANNELS_PER_HALF; ++chn) {
                    m_mapping[i][j][k][chn].first = -1;
                    m_mapping[i][j][k][chn].second = -1;
                }
            }
        }
    }
}

void ChannelMapping::LoadMapping(std::string path) {
	if (not std::filesystem::exists(path)) {
        spdlog::error("Channel mapping file {} does not exist or can't be opened", path);
        return;
    }

    InitMapping();
    spdlog::info("Loading new channel mapping from {}", path);
    std::ifstream infile(path);
    std::string line;
    int line_number = 0;
    unsigned int num_initialized_channels = 0;
    std::getline(infile, line); // Header
    while (std::getline(infile, line)) {
        ++line_number;
        std::istringstream iss(line);
        int row, col, vldb, asic, half, chn;
        if ( !(iss >> row >> col >> vldb >> asic >> half >> chn) ) {
            spdlog::warn("Malformed line in channel mapping file (line {})", line_number);
            continue;
        }

        spdlog::debug("Mapping setup: set vldb {} asic {}.{} chn #{} to (row, col) = ({}, {})", 
                      vldb, asic, half, chn, row, col);

        m_mapping[vldb][asic][half][chn].first  = row;
        m_mapping[vldb][asic][half][chn].second = col;
        ++num_initialized_channels;
    }

    spdlog::info("Loaded new channel mapping; {} channels were initialized",
                 num_initialized_channels);

}

std::pair<int, int> ChannelMapping::GetRowCol(int vldb, int asic, int half, int chn) {
    return m_mapping[vldb][asic][half][chn];
}

void ChannelMapping::PrintMapping() {
    spdlog::info("Printing currently loaded channel mapping..");
    spdlog::info("╔ VLDB ASIC.Half #Chn        Row  Col ╗");
    spdlog::info("║                                     ║");
    for (int i = 0; i < g_NUM_VLDB; ++i) {
        for (int j = 0; j < g_NUM_ASIC_PER_VLDB; ++j) {
            for (int k = 0; k < g_NUM_HALVES_PER_ASIC; ++k) {
                for (int chn = 0; chn < g_NUM_CHANNELS_PER_HALF; ++chn) {
                    int r = m_mapping[i][j][k][chn].first;
                    int c = m_mapping[i][j][k][chn].second;
                    if ((r == -1) || (c == -1)) {
                        spdlog::info("╟    {}    {}.{}    #{: <2d}   ──>     N.A.   ║",
                                     i, j, k, chn);
                    }
                    else {
                        spdlog::info("╟    {}    {}.{}    #{: <2d}   ──>   {:02d}   {:02d}  ║",
                                     i, j, k, chn, r, c);
                    }
                }
            }
        }
    }

    spdlog::info("╚═════════════════════════════════════╝");
}
