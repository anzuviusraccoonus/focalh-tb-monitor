#include <TH2D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/SpectralPage.h"

void SpectralPage::Initialize() {
	mp_canvas->Divide(g_NUM_VLDB, 3, 0.01, 0.01);
	
    std::map<int, std::string> m{{0, "ADC"}, {1, "ToT"}, {2, "ToA"}};
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        for (int v = 0; v < 3; ++v) {
            mp_canvas->cd((vldb + g_NUM_VLDB * v) + 1);
            int num_channels = g_NUM_ASIC_PER_VLDB * g_NUM_HALVES_PER_ASIC * g_NUM_CHANNELS_PER_HALF;
            TH2D* p_graph = new TH2D(Form("VLDB %d %s", vldb, m[v].c_str()),
                                     Form("VLDB %d %s", vldb, m[v].c_str()),
                                     num_channels, -0.5, num_channels + 0.5,
                                     1024, 0, 1023);
            p_graph->SetStats(0);
            p_graph->GetXaxis()->SetTitle("VLDB Channel #");
            p_graph->GetYaxis()->SetTitle(Form("%s", m[v].c_str()));
            p_graph->Draw("COLZ");
            gPad->SetLogz();
            m_objects.push_back(p_graph);
        }
    }
}

void SpectralPage::Update() {
    DataReader* p_reader = DataReader::GetInstance();
    std::vector<ChannelData> buffer_adc = p_reader->buffer_adc;
    std::vector<ChannelData> buffer_tot = p_reader->buffer_tot;
    std::vector<ChannelData> buffer_toa = p_reader->buffer_toa;
    while (m_buffer_idx < buffer_adc.size()) {
        ChannelData data = buffer_adc.at(m_buffer_idx);
        if (data.mg == 4) {
            TH2D* p_graph = static_cast<TH2D*>(m_objects[3 * data.vldb_id]);
            p_graph->Fill(data.chn + 
                          (data.half * g_NUM_CHANNELS_PER_HALF) + 
                          (data.asic_id * g_NUM_HALVES_PER_ASIC * g_NUM_CHANNELS_PER_HALF), 
                          data.value);
        }

        data = buffer_tot.at(m_buffer_idx);
        if (data.mg == 4) {
            TH2D* p_graph = static_cast<TH2D*>(m_objects[3 * data.vldb_id + 1]);
            p_graph->Fill(data.chn + 
                          (data.half * g_NUM_CHANNELS_PER_HALF) + 
                          (data.asic_id * g_NUM_HALVES_PER_ASIC * g_NUM_CHANNELS_PER_HALF), 
                          data.value);
        }

        data = buffer_toa.at(m_buffer_idx);
        if (data.mg == 4) {
            TH2D* p_graph = static_cast<TH2D*>(m_objects[3 * data.vldb_id + 2]);
            p_graph->Fill(data.chn + 
                          (data.half * g_NUM_CHANNELS_PER_HALF) + 
                          (data.asic_id * g_NUM_HALVES_PER_ASIC * g_NUM_CHANNELS_PER_HALF), 
                          data.value);
        }

        ++m_buffer_idx;
    }
}

void SpectralPage::Clear() {
	for (TObject* p_obj : m_objects) {
		static_cast<TH2D*>(p_obj)->Reset();
	}
}

void SpectralPage::Reset() {
    m_buffer_idx = 0;
}
