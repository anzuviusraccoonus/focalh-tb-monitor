#include <TH2D.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/TSPage2.h"

void TSPage2::Initialize() {
    double xpts[g_timegraphs_num_points];
    double ypts[g_timegraphs_num_points];
    for (unsigned int i = 0; i < g_timegraphs_num_points; ++i) {
        xpts[i] = -g_timegraphs_window_seconds + (g_timegraphs_window_seconds / g_timegraphs_num_points) * i;
        ypts[i] = 0;
    }

	mp_canvas->Divide(2, 3, 0.02, 0.02);

    mp_canvas->cd(1);
    TGraph* p_graph_data_read = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_data_read");
    p_graph_data_read->SetTitle("Total Amount of Data Read");
    p_graph_data_read->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_data_read->GetYaxis()->SetTitle("Data Read [bytes]");
    p_graph_data_read->Draw();
    m_objects.push_back(p_graph_data_read);

    mp_canvas->cd(2);
    //for (unsigned int i = 0; i < g_timegraphs_num_points; ++i) {
    //    ypts[i] = 1;
    //}
    //TGraph* p_graph_bx_diff = new TGraph(g_timegraphs_num_points, xpts, ypts);
    //gPad->SetName("p_graph_bx_diff");
    //p_graph_bx_diff->SetTitle("Average difference between sequential lines' BxCnts in time window");
    //p_graph_bx_diff->GetXaxis()->SetTitle("Time [sec.]");
    //p_graph_bx_diff->GetYaxis()->SetTitle("BxCnt(n) - BxCnt(n-1)");
    //p_graph_bx_diff->Draw();
    TH1I* p_graph_bx_diff = new TH1I("BxDiff", "BxDiff", 100, -49, 49);
    p_graph_bx_diff->SetTitle("Difference between sequential lines' Bx counters");
    p_graph_bx_diff->GetXaxis()->SetTitle("Difference");
    p_graph_bx_diff->GetYaxis()->SetTitle("Frequency");
    gPad->SetLogy();
    p_graph_bx_diff->Draw();
    m_objects.push_back(p_graph_bx_diff);

    mp_canvas->cd(3);
    for (unsigned int i = 0; i < g_timegraphs_num_points; ++i) {
        ypts[i] = 0;
    }
    TGraph* p_graph_complete_events = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_complete_events");
    p_graph_complete_events->SetTitle("Number of Complete Events Recorded");
    p_graph_complete_events->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_complete_events->GetYaxis()->SetTitle("Complete Events");
    p_graph_complete_events->Draw();
    m_objects.push_back(p_graph_complete_events);

    mp_canvas->cd(4);
    TGraph* p_graph_incomplete_events = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_incomplete_events");
    p_graph_incomplete_events->SetTitle("Number of Incomplete Events Recorded");
    p_graph_incomplete_events->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_incomplete_events->GetYaxis()->SetTitle("Incomplete Events");
    p_graph_incomplete_events->Draw();
    m_objects.push_back(p_graph_incomplete_events);

    mp_canvas->cd(5);
    TGraph* p_graph_good_triggers = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_good_triggers");
    p_graph_good_triggers->SetTitle("Number of Good Triggers");
    p_graph_good_triggers->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_good_triggers->GetYaxis()->SetTitle("Triggers");
    p_graph_good_triggers->Draw();
    m_objects.push_back(p_graph_good_triggers);

    mp_canvas->cd(6);
    TGraph* p_graph_bad_triggers = new TGraph(g_timegraphs_num_points, xpts, ypts);
    gPad->SetName("p_graph_bad_triggers");
    p_graph_bad_triggers->SetTitle("Number of Bad Triggers");
    p_graph_bad_triggers->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_bad_triggers->GetYaxis()->SetTitle("Triggers");
    p_graph_bad_triggers->Draw();
    m_objects.push_back(p_graph_bad_triggers);

}

void TSPage2::Update() {
	float update_interval = PageManager::GetInstance()->GetUpdateInterval() / 1000.;
    
	DataReader* p_reader = DataReader::GetInstance();
    double xmin, ymin, xmax, ymax;

    TGraph* p_graph_data_read = static_cast<TGraph*>(m_objects[0]);
    unsigned int data_read = p_reader->bytes_read;
	while (p_graph_data_read->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_data_read->RemovePoint(0);
	}
    p_graph_data_read->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_data_read->AddPoint(0, data_read);
    p_graph_data_read->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_data_read->SetMaximum(ymax * 1.02);

    TH1I* p_graph_bx_diff = static_cast<TH1I*>(m_objects[1]);
    for (unsigned int bx : p_reader->buffer_bx_counters) {
        p_graph_bx_diff->Fill(bx);
    }

    p_reader->buffer_bx_counters.clear();

    //TGraph* p_graph_bx_diff = static_cast<TGraph*>(m_objects[1]);
    //std::vector<int> bx_history = p_reader->buffer_bx_counters;
    //double bx_diff_avg = 0;
    //for (int v : bx_history) {
    //    bx_diff_avg += v;
    //}

    //bx_diff_avg /= bx_history.size();
    //bx_history.clear();

	//while (p_graph_bx_diff->GetPointX(0) < -g_timegraphs_window_seconds) {
    //	p_graph_bx_diff->RemovePoint(0);
	//}
    //p_graph_bx_diff->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    //p_graph_bx_diff->AddPoint(0, bx_diff_avg);
    //p_graph_bx_diff->ComputeRange(xmin, ymin, xmax, ymax);
    //p_graph_bx_diff->SetMaximum(ymax * 1.02);
    //p_graph_bx_diff->SetMinimum(ymin * 0.98);

    TGraph* p_graph_complete_events = static_cast<TGraph*>(m_objects[2]);
    int num_complete_events = p_reader->num_complete_events;
	while (p_graph_complete_events->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_complete_events->RemovePoint(0);
	}
    p_graph_complete_events->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_complete_events->AddPoint(0, num_complete_events);
    p_graph_complete_events->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_complete_events->SetMaximum(ymax * 1.02);

    TGraph* p_graph_incomplete_events = static_cast<TGraph*>(m_objects[3]);
    int num_incomplete_events = p_reader->num_incomplete_events;
	while (p_graph_incomplete_events->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_incomplete_events->RemovePoint(0);
	}
    p_graph_incomplete_events->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_incomplete_events->AddPoint(0, num_incomplete_events);
    p_graph_incomplete_events->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_incomplete_events->SetMaximum(ymax * 1.02);

    TGraph* p_graph_good_triggers = static_cast<TGraph*>(m_objects[4]);
    int num_good_triggers = p_reader->num_good_triggers;
	while (p_graph_good_triggers->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_good_triggers->RemovePoint(0);
	}
    p_graph_good_triggers->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_good_triggers->AddPoint(0, num_good_triggers);
    p_graph_good_triggers->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_good_triggers->SetMaximum(ymax * 1.02);

    TGraph* p_graph_bad_triggers = static_cast<TGraph*>(m_objects[5]);
    int num_bad_triggers = p_reader->num_bad_triggers;
	while (p_graph_bad_triggers->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_bad_triggers->RemovePoint(0);
	}
    p_graph_bad_triggers->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_bad_triggers->AddPoint(0, num_bad_triggers);
    p_graph_bad_triggers->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_bad_triggers->SetMaximum(ymax * 1.02);

}

void TSPage2::Clear() { 
	static_cast<TH1I*>(m_objects[1])->Reset();
}

void TSPage2::Reset() {
}
