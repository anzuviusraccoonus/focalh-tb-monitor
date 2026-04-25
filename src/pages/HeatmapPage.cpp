#include <TH2D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "ChannelMapping.h"
#include "globals.h"

#include "pages/HeatmapPage.h"

void HeatmapPage::Initialize() {
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
    std::vector<ChannelData> buffer = *(static_cast<std::vector<ChannelData>*>(mp_data));
    float temp[g_NUM_MACHINE_GUN_TRIGGERS][g_HEATMAP_NUM_ROWS][g_HEATMAP_NUM_COLS] = {};
 
    int delta_events = ((p_reader->num_complete_events + p_reader->num_incomplete_events) - m_num_events);
    if (delta_events == 0) {
        return;
    }

    while (m_buffer_idx < buffer.size()) {
        ChannelData data = buffer.at(m_buffer_idx);
        coords = p_mapping->GetRowCol(data.vldb_id, data.asic_id, data.half, data.chn);
        temp[data.mg-1][coords.first][coords.second] += data.value;

        ++m_buffer_idx;
    }

    for (int mg = 0; mg < g_NUM_MACHINE_GUN_TRIGGERS; ++mg) {
        TH2D* p_graph = static_cast<TH2D*>(m_objects[mg]);
        for (int x = 0; x < g_HEATMAP_NUM_COLS; ++x) {
            for (int y = 0; y < g_HEATMAP_NUM_ROWS; ++y) {
                p_graph->SetBinContent(x+1, y+1, (p_graph->GetBinContent(x+1, y+1) + temp[mg][y][x]) / delta_events);
            }
        }
    }    

    m_num_events = p_reader->num_complete_events + p_reader->num_incomplete_events;
}

void HeatmapPage::Clear() {
    m_num_events = 0;
	for (TObject* p_obj : m_objects) {
		static_cast<TH2D*>(p_obj)->Reset();
	}
}

void HeatmapPage::Reset() {
    m_num_events = 0;
    m_buffer_idx = 0;
}
