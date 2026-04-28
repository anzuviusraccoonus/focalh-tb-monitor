#include <TH2D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "ChannelMapping.h"
#include "globals.h"

#include "pages/HeatmapPage.h"

void HeatmapPage::Initialize() {
    Reset();
	mp_canvas->DivideSquare(g_NUM_MACHINE_GUN_TRIGGERS, 0.001, 0.001);
    for (int n = 0; n < g_NUM_MACHINE_GUN_TRIGGERS; ++n) {
        mp_canvas->cd(n+1);
        TH2D* p_graph = new TH2D(Form("%s_%d", mp_canvas->GetName(), n),
                                 Form("Machine Gun Trigger %d", n),
                                 g_HEATMAP_NUM_COLS, -0.5, g_HEATMAP_NUM_COLS - 0.5,
                                 g_HEATMAP_NUM_ROWS, -0.5, g_HEATMAP_NUM_ROWS - 0.5);

        p_graph->SetStats(0);
        p_graph->Draw("COLZ");
        gPad->SetLogz();
        m_objects.push_back(p_graph);
    }

    m_num_events = 0;
}

void HeatmapPage::Update() {
    DataReader* p_reader = DataReader::GetInstance();
	ChannelMapping* p_mapping = ChannelMapping::GetInstance();
	
    std::pair<int, int> coords;
    std::vector<ChannelData>* buffer = static_cast<std::vector<ChannelData>*>(mp_data);
 
    int total_current_events = 0;
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        int vldb_events = p_reader->num_complete_events[vldb] + p_reader->num_incomplete_events[vldb];
        total_current_events = std::max(total_current_events, vldb_events);
    }

    int delta_events = total_current_events - m_num_events;
    if (delta_events == 0) {
        return;
    }

    // This is the temporary storage for calculation of new averages per channel,
    // so we make sure it's zeroed every time, before counting sums
    std::memset(m_temp_heatmap_data, 0, sizeof m_temp_heatmap_data);
    for (const ChannelData& data : *buffer) {
        coords = p_mapping->GetRowCol(data.vldb_id, data.asic_id, data.half, data.chn);
        if ((coords.first == -1) || (coords.second == -1)) {
            continue;
        }

        if (data.mg > g_NUM_MACHINE_GUN_TRIGGERS) {
            continue;
        }

        m_temp_heatmap_data[data.mg-1][coords.first][coords.second] += data.value;
    }

    for (int mg = 0; mg < g_NUM_MACHINE_GUN_TRIGGERS; ++mg) {
        TH2D* p_graph = static_cast<TH2D*>(m_objects[mg]);
        for (int x = 0; x < g_HEATMAP_NUM_COLS; ++x) {
            for (int y = 0; y < g_HEATMAP_NUM_ROWS; ++y) {
                double previous = p_graph->GetBinContent(x+1, y+1) * m_num_events; // "unaveraged"
                p_graph->SetBinContent(x+1, y+1, (previous + m_temp_heatmap_data[mg][y][x])
                                                  / (m_num_events + delta_events) );
            }
        }
    }    

    m_num_events = total_current_events;
}

void HeatmapPage::Clear() {
    m_num_events = 0;
	for (TObject* p_obj : m_objects) {
		static_cast<TH2D*>(p_obj)->Reset();
	}
}

void HeatmapPage::Reset() {
    m_num_events = 0;
}
