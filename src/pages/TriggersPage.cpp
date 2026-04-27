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

#include "pages/TriggersPage.h"

void TriggersPage::Initialize() {
    double xpts[g_timegraphs_num_points];
    double ypts[g_timegraphs_num_points];
    for (unsigned int i = 0; i < g_timegraphs_num_points; ++i) {
        xpts[i] = -g_timegraphs_window_seconds + (g_timegraphs_window_seconds / g_timegraphs_num_points) * i;
        ypts[i] = 0;
    }

	mp_canvas->Divide(2, 2, 0.001, 0.001);

    mp_canvas->cd(1);
    TGraph* p_graph_good_triggers = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_good_triggers");
    p_graph_good_triggers->SetTitle("Total Number of Good Triggers");
    p_graph_good_triggers->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_good_triggers->GetYaxis()->SetTitle("Triggers");
    p_graph_good_triggers->Draw();
    m_objects.push_back(p_graph_good_triggers);

    mp_canvas->cd(2);
    TGraph* p_graph_good_trigger_rate = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_good_trigger_rate");
    p_graph_good_trigger_rate->SetTitle("Good Trigger Rate");
    p_graph_good_trigger_rate->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_good_trigger_rate->GetYaxis()->SetTitle("Triggers Rate");
    p_graph_good_trigger_rate->Draw();
    m_objects.push_back(p_graph_good_trigger_rate);

    mp_canvas->cd(3);
    TGraph* p_graph_bad_triggers = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_bad_triggers");
    p_graph_bad_triggers->SetTitle("Total Number of Bad Triggers");
    p_graph_bad_triggers->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_bad_triggers->GetYaxis()->SetTitle("Triggers");
    p_graph_bad_triggers->Draw();
    m_objects.push_back(p_graph_bad_triggers);

    mp_canvas->cd(4);
    TGraph* p_graph_bad_trigger_rate = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_bad_trigger_rate");
    p_graph_bad_trigger_rate->SetTitle("Bad Trigger Rate");
    p_graph_bad_trigger_rate->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_bad_trigger_rate->GetYaxis()->SetTitle("Trigger Rate");
    p_graph_bad_trigger_rate->Draw();
    m_objects.push_back(p_graph_bad_trigger_rate);

}

void TriggersPage::Update() {
	DataReader* p_reader = DataReader::GetInstance();
    long int current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    long int last_update_dt = current_time - PageManager::GetInstance()->GetLastUpdatedTime();
    double xmin, ymin, xmax, ymax;

    for (int i = 0; i < 4; ++i) {
        TGraph* p_graph = static_cast<TGraph*>(m_objects[i]);
        p_graph->MovePoints(-(last_update_dt / 1000.), 0.);
        while (p_graph->GetPointX(0) < -g_timegraphs_window_seconds) {
            p_graph->RemovePoint(0);
        }

        if (i == 0) {
            p_graph->AddPoint(0, p_reader->num_good_triggers);
        }

        if (i == 1) {
            int num_good_triggers = p_reader->num_good_triggers;
            int delta = num_good_triggers - m_num_good_triggers_last;
            p_graph->AddPoint(0, delta);
            m_num_good_triggers_last = num_good_triggers;
        }

        if (i == 2) {
            p_graph->AddPoint(0, p_reader->num_bad_triggers);
        }

        if (i == 3) {
            int num_bad_triggers = p_reader->num_bad_triggers;
            int delta = num_bad_triggers - m_num_bad_triggers_last;
            p_graph->AddPoint(0, delta);
            m_num_bad_triggers_last = num_bad_triggers;
        }

        p_graph->ComputeRange(xmin, ymin, xmax, ymax);
        p_graph->SetMaximum(ymax * 1.05);
    }

}

void TriggersPage::Clear() { 
}

void TriggersPage::Reset() {
    m_num_good_triggers_last = 0;
    m_num_bad_triggers_last = 0;
}
