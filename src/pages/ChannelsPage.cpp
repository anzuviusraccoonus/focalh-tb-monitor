#include <TH2D.h>
#include <TStyle.h>

#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/ChannelsPage.h"

void ChannelsPage::Initialize() {
    m_buffer_idx = 0;
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
	while (m_buffer_idx < p_buffer->size()) {
		unsigned int vldb_id = p_buffer->at(m_buffer_idx).vldb_id;
		unsigned int asic_id = p_buffer->at(m_buffer_idx).asic_id;
		unsigned int half    = p_buffer->at(m_buffer_idx).half;
		if ( (vldb_id == m_vldb) &
			 (asic_id == m_asic) &
			 (half    == m_half)   ) {

			unsigned int chn    = p_buffer->at(m_buffer_idx).chn;
			unsigned int value  = p_buffer->at(m_buffer_idx).value;
			unsigned int mg     = p_buffer->at(m_buffer_idx).mg;
			TH2D* p_graph = static_cast<TH2D*>(m_objects[chn]);
			p_graph->Fill(mg, value);
		}

		++m_buffer_idx;
	}
}

void ChannelsPage::Clear() {
	for (TObject* p_obj : m_objects) {
		static_cast<TH2D*>(p_obj)->Reset();
	}
}

void ChannelsPage::Reset() {
    m_buffer_idx = 0;
}

void ChannelsPage::SetBoardIDs(unsigned int vldb_id, unsigned int asic_id, unsigned int half) {
	m_vldb = vldb_id;
	m_asic = asic_id;
	m_half = half;
}
