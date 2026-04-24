#include <TH2D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "ChannelMapping.h"
#include "globals.h"

#include "pages/HeatmapPage.h"

void HeatmapPage::Initialize() {
	mp_canvas->DivideSquare(g_NUM_MACHINE_GUN_TRIGGERS, 0.01, 0.01);
    for (int n = 0; n < g_NUM_MACHINE_GUN_TRIGGERS; ++n) {
        mp_canvas->cd(n+1);
        TH2D* p_graph = new TH2D(Form("%s_%d", mp_canvas->GetName(), n),
                                 Form("Machine Gun Trigger %d", n),
                                 g_HEATMAP_NUM_COLS, -0.5, g_HEATMAP_NUM_COLS - 0.5,
                                 g_HEATMAP_NUM_ROWS, -0.5, g_HEATMAP_NUM_ROWS - 0.5);

        p_graph->SetStats(0);
        p_graph->Draw("COLZ");
        p_graph->SetMaximum(1023);
        gPad->SetLogz();
        m_objects.push_back(p_graph);
    }
}

void HeatmapPage::Update() {
    DataReader* p_reader = DataReader::GetInstance();
	ChannelMapping* p_mapping = ChannelMapping::GetInstance();

    std::vector<ChannelData> buffer_adc = *(static_cast<std::vector<ChannelData>*>(mp_data));
	int num_complete_events = p_reader->num_complete_events;
	int num_incomplete_events = p_reader->num_incomplete_events;
	std::pair<int, int> coords;
    while (m_buffer_idx < buffer_adc.size()) {
        ChannelData data = buffer_adc.at(m_buffer_idx);
		coords = p_mapping->GetRowCol(data.vldb_id, data.asic_id, data.half, data.chn);
		if (not ((coords.first == -1) || (coords.second == -1))) {
			TH2D* p_graph = static_cast<TH2D*>(m_objects[data.mg]);
			double previous_bin_content = p_graph->GetBinContent(coords.first + 1, coords.second + 1);
			p_graph->SetBinContent(coords.first + 1, coords.second + 1, 
								   previous_bin_content + 
								   (data.value / 
								   (num_complete_events + num_incomplete_events + 1)));
		}

        ++m_buffer_idx;
    }
}

void HeatmapPage::Clear() {
	for (TObject* p_obj : m_objects) {
		static_cast<TH2D*>(p_obj)->Reset();
	}
}

void HeatmapPage::Reset() {
    m_buffer_idx = 0;
}
