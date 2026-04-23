#include <TH2D.h>
#include <TGraph.h>
#include <TH1D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/TSPage1.h"

void TSPage1::Initialize() {
    double xpts[g_timegraphs_num_points];
    double ypts[g_timegraphs_num_points];
    for (unsigned int i = 0; i < g_timegraphs_num_points; ++i) {
        xpts[i] = -g_timegraphs_window_seconds + (g_timegraphs_window_seconds / g_timegraphs_num_points) * i;
        ypts[i] = 0;
    }

	mp_canvas->Divide(2, 3, 0.02, 0.02);

    mp_canvas->cd(1);
    TGraph* p_graph_timestamps = new TGraph(g_timegraphs_num_points, xpts, ypts);
    p_graph_timestamps->SetTitle("Timestamps of Start of Events (MG 0)");
    p_graph_timestamps->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_timestamps->GetYaxis()->SetTitle("Event Timestamp");
    p_graph_timestamps->Draw();
    m_objects.push_back(p_graph_timestamps);

    mp_canvas->cd(2);
    TH1D* p_graph_mg_triggers = new TH1D("MGprTrg", "Machine Gun Triggers per Event",
                               g_NUM_MACHINE_GUN_TRIGGERS, 0.5, g_NUM_MACHINE_GUN_TRIGGERS + 0.5);
    p_graph_mg_triggers->SetXTitle("# of machine gun triggers in event");
    p_graph_mg_triggers->SetTitle("# of Machine Gun Triggers per Event");
    p_graph_mg_triggers->Draw();
    m_objects.push_back(p_graph_mg_triggers);

    mp_canvas->cd(3);
    TGraph* p_graph_complete_events = new TGraph(g_timegraphs_num_points, xpts, ypts);
    p_graph_complete_events->SetTitle("Complete Event Rate");
    p_graph_complete_events->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_complete_events->GetYaxis()->SetTitle("Complete Event Rate [events/sec.]");
    p_graph_complete_events->Draw();
    m_objects.push_back(p_graph_complete_events);
    
    mp_canvas->cd(4);
    TGraph* p_graph_incomplete_events = new TGraph(g_timegraphs_num_points, xpts, ypts);
    p_graph_incomplete_events->SetTitle("Incomplete Event Rate");
    p_graph_incomplete_events->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_incomplete_events->GetYaxis()->SetTitle("Incomplete Event Rate [events/sec.]");
    p_graph_incomplete_events->Draw();
    m_objects.push_back(p_graph_incomplete_events);
    
    mp_canvas->cd(5);
    TGraph* p_graph_good_triggers = new TGraph(g_timegraphs_num_points, xpts, ypts);
    p_graph_good_triggers->SetTitle("Good Triggers Rate");
    p_graph_good_triggers->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_good_triggers->GetYaxis()->SetTitle("Complete Event Rate [events/sec.]");
    p_graph_good_triggers->Draw();
    m_objects.push_back(p_graph_good_triggers);
    
    mp_canvas->cd(6);
    TGraph* p_graph_bad_triggers = new TGraph(g_timegraphs_num_points, xpts, ypts);
    p_graph_bad_triggers->SetTitle("Bad Triggers Rate");
    p_graph_bad_triggers->GetXaxis()->SetTitle("Time [sec.]");
    p_graph_bad_triggers->GetYaxis()->SetTitle("Incomplete Event Rate [events/sec.]");
    p_graph_bad_triggers->Draw();
    m_objects.push_back(p_graph_bad_triggers);
}

void TSPage1::Update() {
	float update_interval = PageManager::GetInstance()->GetUpdateInterval() / 1000.;
    
	DataReader* p_reader = DataReader::GetInstance();
    long int start_time = p_reader->frame_start_time;

    double xmin, ymin, xmax, ymax;

    TGraph* p_graph_timestamps = static_cast<TGraph*>(m_objects[0]);
	while (p_graph_timestamps->GetPointX(0) < -g_timegraphs_window_seconds) {
	    p_graph_timestamps->RemovePoint(0);
	}
    p_graph_timestamps->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_timestamps->AddPoint(0, start_time);
    p_graph_timestamps->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_timestamps->SetMaximum(ymax * 1.02);

    int num_complete_events = p_reader->num_complete_events;
    int num_complete_events_dt = num_complete_events - m_num_complete_events;
    m_num_complete_events = num_complete_events;
    TGraph* p_graph_complete_events = static_cast<TGraph*>(m_objects[2]);
	while (p_graph_complete_events->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_complete_events->RemovePoint(0);
	}
    p_graph_complete_events->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_complete_events->AddPoint(0, num_complete_events_dt);
    p_graph_complete_events->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_complete_events->SetMaximum(ymax * 1.02);
    
    int num_incomplete_events = p_reader->num_incomplete_events;
    int num_incomplete_events_dt = num_incomplete_events - m_num_incomplete_events;
    m_num_incomplete_events = num_incomplete_events;
    TGraph* p_graph_incomplete_events = static_cast<TGraph*>(m_objects[3]);
	while (p_graph_incomplete_events->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_incomplete_events->RemovePoint(0);
	}
    p_graph_incomplete_events->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_incomplete_events->AddPoint(0, num_incomplete_events_dt);
    p_graph_incomplete_events->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_incomplete_events->SetMaximum(ymax * 1.02);

    int num_good_triggers = p_reader->num_good_triggers;
    int num_good_triggers_dt = num_good_triggers - m_num_good_triggers;
    m_num_good_triggers = num_good_triggers;
    TGraph* p_graph_good_triggers = static_cast<TGraph*>(m_objects[4]);
	while (p_graph_good_triggers->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_good_triggers->RemovePoint(0);
	}
    p_graph_good_triggers->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_good_triggers->AddPoint(0, num_good_triggers_dt);
    p_graph_good_triggers->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_good_triggers->SetMaximum(ymax * 1.02);
    
    int num_bad_triggers = p_reader->num_bad_triggers;
    int num_bad_triggers_dt = num_bad_triggers - m_num_bad_triggers;
    m_num_bad_triggers = num_bad_triggers;
    TGraph* p_graph_bad_triggers = static_cast<TGraph*>(m_objects[5]);
	while (p_graph_bad_triggers->GetPointX(0) < -g_timegraphs_window_seconds) {
    	p_graph_bad_triggers->RemovePoint(0);
	}
    p_graph_bad_triggers->MovePoints(-(g_timegraphs_window_seconds / g_timegraphs_num_points) * update_interval, 0.);
    p_graph_bad_triggers->AddPoint(0, num_bad_triggers_dt);
    p_graph_bad_triggers->ComputeRange(xmin, ymin, xmax, ymax);
    p_graph_bad_triggers->SetMaximum(ymax * 1.02);
    
    TH1D* p_graph_machineguns = static_cast<TH1D*>(m_objects[1]);
    std::vector<int> buffer_machineguns = p_reader->buffer_machineguns;
    while (m_buffer_idx < buffer_machineguns.size()) {
        p_graph_machineguns->Fill(buffer_machineguns.at(m_buffer_idx));
        ++m_buffer_idx;
    }
}

void TSPage1::Clear() {
	static_cast<TH1D*>(m_objects[1])->Reset();
}

void TSPage1::Reset() {
    m_num_complete_events = 0;
    m_num_incomplete_events = 0;
    m_num_good_triggers = 0;
    m_num_bad_triggers = 0;
    m_buffer_idx = 0;
}
