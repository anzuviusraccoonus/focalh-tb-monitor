#include "DataReader.h"
#include "PageManager.h"
#include "DataStructs.h"
#include "globals.h"
#include "pages/ChannelsPage.h"
#include "pages/TSPage1.h"
#include "pages/TSPage2.h"
#include "pages/SpectralPage.h"
#include "pages/HeatmapPage.h"
#include "pages/StatusPage.h"

void BuildPages() {
    spdlog::info("Building pages...");
    DataReader*  p_reader      = DataReader::GetInstance();
    PageManager* p_pagemanager = PageManager::GetInstance();

    std::map<unsigned int, std::string> m{{0, "ADC"}, {1, "ToT"}, {2, "ToA"}};
    std::map<unsigned int, std::vector<ChannelData>*> t{ {0, &(p_reader->buffer_adc)}, 
                                                         {1, &(p_reader->buffer_tot)}, 
                                                         {2, &(p_reader->buffer_toa)} };
    for (unsigned int v = 0; v < 3; ++v) {
        for (unsigned int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
            for (unsigned int asic = 0; asic < g_NUM_ASIC_PER_VLDB; ++asic) {
                for (unsigned int half = 0; half < g_NUM_HALVES_PER_ASIC; ++half) {
                    ChannelsPage* page = new ChannelsPage(Form("vldb%d_asic%d.%d_%s", vldb, asic, half, m[v].c_str()),
                                                          Form("vldb%d_asic%d.%d_%s", vldb, asic, half, m[v].c_str()),
                                                          t[v], 
                                                          Form("/VLDB_%d/ASIC_%d/%d", vldb, asic, half) );
                    page->SetBoardIDs(vldb, asic, half);
                    p_pagemanager->AddPage(page);
                }
            }
        }

        HeatmapPage* p_heatmappage = new HeatmapPage(Form("%s_heatmap", m[v].c_str()),
                                                     Form("%s_heatmap", m[v].c_str()),
                                                     t[v],
                                                     "/Heatmaps");
        p_pagemanager->AddPage(p_heatmappage);
    }

	StatusPage* p_statuspage = new StatusPage("Overview",
											  "Overview",
											  nullptr, "/");
	p_pagemanager->AddPage(p_statuspage);

    TSPage1* p_tspage1 = new TSPage1("TS Page 1", 
											  "TS1", 
											  &(p_reader->frame_start_time), "/");
    p_pagemanager->AddPage(p_tspage1);

    TSPage2* p_tspage2 = new TSPage2("TS Page 2", 
									 "TS2", 
									 nullptr, "/");
    p_pagemanager->AddPage(p_tspage2);

    SpectralPage* p_spectralpage = new SpectralPage("Spectral Graphs", 
												    "Spectral", 
												    nullptr, "/");
    p_pagemanager->AddPage(p_spectralpage);

    spdlog::info("Finished building pages");
}
