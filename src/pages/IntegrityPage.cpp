#include <chrono>
#include <TH2D.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/IntegrityPage.h"

void IntegrityPage::Initialize() {
    double xpts[g_timegraphs_num_points];
    double ypts[g_timegraphs_num_points];
    for (unsigned int i = 0; i < g_timegraphs_num_points; ++i) {
        xpts[i] = -g_timegraphs_window_seconds + (g_timegraphs_window_seconds / g_timegraphs_num_points) * i;
        ypts[i] = 0;
    }

	mp_canvas->Divide(g_NUM_ASIC_PER_VLDB, g_NUM_VLDB, 0.001, 0.001);
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        for (int asic = 0; asic < g_NUM_ASIC_PER_VLDB; ++asic) {
            mp_canvas->cd(1 + (2 * vldb) + asic);
            TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
            p_graph->SetTitle(Form("VLDB %d ASIC %d Data Errors", vldb, asic));
            p_graph->GetXaxis()->SetTitle("Time [sec.]");
            p_graph->GetYaxis()->SetTitle("Errors");
            p_graph->Draw();
            m_objects.push_back(p_graph);
        }
    }
}

void IntegrityPage::Update() { 
	DataReader* p_reader = DataReader::GetInstance();
    long int current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    long int last_update_dt = current_time - PageManager::GetInstance()->GetLastUpdatedTime();
    double xmin, ymin, xmax, ymax;

    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        for (int asic = 0; asic < g_NUM_ASIC_PER_VLDB; ++asic) {
            TGraph* p_graph = static_cast<TGraph*>(m_objects[2 * vldb + asic]);
            p_graph->MovePoints(-(last_update_dt / 1000.), 0.);
            while (p_graph->GetPointX(0) < -g_timegraphs_window_seconds) {
                p_graph->RemovePoint(0);
            }
            p_graph->AddPoint(0, p_reader->error_counts[vldb][asic]);
            p_graph->ComputeRange(xmin, ymin, xmax, ymax);
            p_graph->SetMaximum(ymax * 1.02);
        }
    }
}

void IntegrityPage::Clear() {
}

void IntegrityPage::Reset() {
}
