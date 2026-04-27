#include "DataReader.h"
#include "PageManager.h"
#include "DataStructs.h"
#include "globals.h"
#include "pages/ChannelsPage.h"
#include "pages/SpectralPage.h"
#include "pages/HeatmapPage.h"
#include "pages/StatusPage.h"
#include "pages/SummaryPage.h"
#include "pages/IntegrityPage.h"
#include "pages/EventsPage.h"
#include "pages/TriggersPage.h"

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

        HeatmapPage* p_heatmappage = new HeatmapPage(Form("%s Heatmap", m[v].c_str()),
                                                     Form("%s_heatmap", m[v].c_str()),
                                                     t[v],
                                                     "/Heatmaps");
        p_pagemanager->AddPage(p_heatmappage);
    }

	StatusPage* p_statuspage = new StatusPage("Overview Page",
											  "Overview",
											  nullptr, "/");
	p_pagemanager->AddPage(p_statuspage);

    EventsPage* p_eventspage = new EventsPage("Event Statistics",
                                              "EventsPage",
                                              nullptr, "/");
    p_pagemanager->AddPage(p_eventspage);
    
    TriggersPage* p_triggerspage = new TriggersPage("Trigger Statistics",
                                                    "TriggerPage",
                                                    nullptr, "/");
    p_pagemanager->AddPage(p_triggerspage);

    SpectralPage* p_spectralpage = new SpectralPage("Spectral Graphs", 
												    "SpectralPage", 
												    nullptr, "/");
    p_pagemanager->AddPage(p_spectralpage);

    // Disabled until it's updated to handle separate ADC counts for VLDB links
    //SummaryPage* p_summarypage = new SummaryPage("ADC Summary Page",
    //                                             "ADCSummary",
    //                                              nullptr, "/");

    //p_pagemanager->AddPage(p_summarypage);

    IntegrityPage* p_integritypage = new IntegrityPage("Data Integrity Errors",
                                                       "Integrity",
                                                       nullptr, "/");

    p_pagemanager->AddPage(p_integritypage);

    spdlog::info("Finished building pages");
}
