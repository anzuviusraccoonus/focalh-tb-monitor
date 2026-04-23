#include <TH2D.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/SummaryPage.h"

void SummaryPage::Initialize() {
    Reset();
    TH1D* p_graph_adc = new TH1D("ADC", "ADC", 256, 0., 100.);
    p_graph_adc->SetTitle("Event ADC Sums (complete events only)");
    p_graph_adc->GetXaxis()->SetTitle("Sum ADC");
    p_graph_adc->GetYaxis()->SetTitle("Frequency");
    p_graph_adc->SetCanExtend(true);
    p_graph_adc->Draw();
    m_objects.push_back(p_graph_adc);
}

void SummaryPage::Update() {
    std::vector<EventData> event_buffer = DataReader::GetInstance()->event_buffer;
    TH1D* graph = static_cast<TH1D*>(m_objects[0]);
    while (m_buffer_idx < event_buffer.size()) {
        EventData data = event_buffer.at(m_buffer_idx);
        if (data.num_machineguns == g_NUM_MACHINE_GUN_TRIGGERS) {
            graph->Fill(data.adc);
        }

        ++m_buffer_idx;
    }
}

void SummaryPage::Clear() {
    static_cast<TH1D*>(m_objects[0])->Reset();
}

void SummaryPage::Reset() {
    m_buffer_idx = 0;
}
