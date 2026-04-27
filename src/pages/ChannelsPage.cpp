#include <TH2D.h>
#include <TStyle.h>

#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/ChannelsPage.h"

void ChannelsPage::Initialize() {
	mp_canvas->DivideSquare(g_NUM_CHANNELS_PER_HALF, 0.001, 0.001);
	for (int i = 0; i < g_NUM_CHANNELS_PER_HALF; ++i) {
		TH2D* p_graph = new TH2D(Form("%s_%d", mp_canvas->GetName(), i),
						       Form("Channel %d", i),
							   g_NUM_MACHINE_GUN_TRIGGERS, 0, g_NUM_MACHINE_GUN_TRIGGERS - 1,
							   128, 0, 1023);

		mp_canvas->cd(i+1);
		p_graph->SetXTitle("Machine Gun Trg. #");
		p_graph->SetYTitle("");
		p_graph->SetLabelSize(0.025, "X");
		p_graph->SetLabelSize(0.025, "Y");
		p_graph->SetTitleSize(0.05, "X");
		p_graph->SetTitleSize(0.05, "Y");
		p_graph->SetStats(0);
		gPad->SetLogz();
		gStyle->SetTitleFont(32, "t");
		p_graph->Draw();
		m_objects.push_back(p_graph);
	}
}

void ChannelsPage::Update() {
	std::vector<ChannelData>* p_buffer = static_cast<std::vector<ChannelData>*>(mp_data);
    for (const ChannelData& data : *p_buffer) {
        unsigned int vldb_id = data.vldb_id;
        unsigned int asic_id = data.asic_id;
        unsigned int half = data.half;
		if ( (vldb_id == m_vldb) &
			 (asic_id == m_asic) &
			 (half    == m_half)   ) {

            unsigned int chn = data.chn;
            unsigned int value = data.value;
            unsigned int mg = data.mg;
            
            // Prevent bad ChannelData objects (= with invalid values) from 
            // crashing the monitor; these problems should be logged by the DataReader
            if ((chn >= m_objects.size()) || (mg > g_NUM_MACHINE_GUN_TRIGGERS)) {
                continue;
            }

			TH2D* p_graph = static_cast<TH2D*>(m_objects[chn]);
			p_graph->Fill(mg, value);
		}
	}
}

void ChannelsPage::Clear() {
	for (TObject* p_obj : m_objects) {
		static_cast<TH2D*>(p_obj)->Reset();
	}
}

void ChannelsPage::Reset() {
}

void ChannelsPage::SetBoardIDs(unsigned int vldb_id, unsigned int asic_id, unsigned int half) {
	m_vldb = vldb_id;
	m_asic = asic_id;
	m_half = half;
}
