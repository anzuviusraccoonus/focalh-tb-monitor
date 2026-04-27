#include <chrono>
#include <TH2D.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TH1D.h>
#include <TStyle.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/EventsPage.h"

void EventsPage::Initialize() {
    double xpts[g_timegraphs_num_points];
    double ypts[g_timegraphs_num_points];
    for (unsigned int i = 0; i < g_timegraphs_num_points; ++i) {
        xpts[i] = -g_timegraphs_window_seconds + (g_timegraphs_window_seconds / g_timegraphs_num_points) * i;
        ypts[i] = 0.;
    }

	mp_canvas->Divide(2, 4, 0.02, 0.02);

    // Having to set it up like this sucks but that one different plot type makes it hella annoying
    // and I am waaaaay too sleep deprived to figure it out

    mp_canvas->cd(1);
    TMultiGraph* p_graph_timestamps = new TMultiGraph("Timestamps", "Timestamps of Events");
    p_graph_timestamps->GetXaxis()->SetTitle("Runtime [sec.]");
    p_graph_timestamps->GetYaxis()->SetTitle("Timestamps");
    TLegend* p_legend = new TLegend();
    p_legend->SetMargin(0.1);
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
        p_graph->SetTitle(Form("VLDB Link %d", vldb));
        p_legend->AddEntry(p_graph);
        p_graph_timestamps->Add(p_graph);
    }
    p_graph_timestamps->Draw("pmc plc");
    p_legend->Draw();
    m_objects.push_back(p_graph_timestamps);


    mp_canvas->cd(2);
    TH1I* p_hist_machineguns = new TH1I("Machineguns", "Machineguns",
                                        g_NUM_MACHINE_GUN_TRIGGERS, 0.5, g_NUM_MACHINE_GUN_TRIGGERS + 0.5);
    gPad->SetLogy();
    p_hist_machineguns->Draw();
    m_objects.push_back(p_hist_machineguns);


    mp_canvas->cd(3);
    TMultiGraph* p_graph_complete_events = new TMultiGraph("CompleteEvents", Form("Number of Events with %d Machine Gun Triggers", g_NUM_MACHINE_GUN_TRIGGERS));
    p_graph_complete_events->GetXaxis()->SetTitle("Runtime [sec.]");
    p_graph_complete_events->GetYaxis()->SetTitle("Events");
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
        p_graph->SetTitle(Form("%d", vldb));
        p_graph_complete_events->Add(p_graph);
    }
    p_graph_complete_events->Draw("pmc plc");
    p_legend->Draw();
    m_objects.push_back(p_graph_complete_events);
    
    
    mp_canvas->cd(4);
    TMultiGraph* p_graph_complete_event_rate = new TMultiGraph("CompleteEventRate", "Rate of Complete Events");
    p_graph_complete_event_rate->GetXaxis()->SetTitle("Runtime [sec.]");
    p_graph_complete_event_rate->GetYaxis()->SetTitle("Event Rate");
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
        p_graph->SetTitle(Form("%d", vldb));
        p_graph_complete_event_rate->Add(p_graph);
    }
    p_graph_complete_event_rate->Draw("pmc plc");
    p_legend->Draw();
    m_objects.push_back(p_graph_complete_event_rate);


    mp_canvas->cd(5);
    TMultiGraph* p_graph_incomplete_events = new TMultiGraph("IncompleteEvents", Form("Number of Events with <%d Machine Gun Triggers", g_NUM_MACHINE_GUN_TRIGGERS));
    p_graph_incomplete_events->GetXaxis()->SetTitle("Runtime [sec.]");
    p_graph_incomplete_events->GetYaxis()->SetTitle("Events");
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
        p_graph->SetTitle(Form("%d", vldb));
        p_graph_incomplete_events->Add(p_graph);
    }
    p_graph_incomplete_events->Draw("pmc plc");
    p_legend->Draw();
    m_objects.push_back(p_graph_incomplete_events);


    mp_canvas->cd(6);
    TMultiGraph* p_graph_incomplete_event_rate = new TMultiGraph("IncompleteEventRate", "Rate of Incomplete Events");
    p_graph_incomplete_event_rate->GetXaxis()->SetTitle("Runtime [sec.]");
    p_graph_incomplete_event_rate->GetYaxis()->SetTitle("Event Rate");
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
        p_graph->SetTitle(Form("%d", vldb));
        p_graph_incomplete_event_rate->Add(p_graph);
    }
    p_graph_incomplete_event_rate->Draw("pmc plc");
    p_legend->Draw();
    m_objects.push_back(p_graph_incomplete_event_rate);
    
    
    mp_canvas->cd(7);
    TMultiGraph* p_graph_error_events = new TMultiGraph("TaintedEvents", "Number of Events with Errors");
    p_graph_error_events->GetXaxis()->SetTitle("Runtime [sec.]");
    p_graph_error_events->GetYaxis()->SetTitle("Events");
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
        p_graph->SetTitle(Form("%d", vldb));
        p_graph_error_events->Add(p_graph);
    }
    p_graph_error_events->Draw("pmc plc");
    p_legend->Draw();
    m_objects.push_back(p_graph_error_events);
   

    mp_canvas->cd(8);
    TMultiGraph* p_graph_error_events_rate = new TMultiGraph("TaintedEventRate", "Rate of Corrupt Events");
    p_graph_error_events_rate->GetXaxis()->SetTitle("Runtime [sec.]");
    p_graph_error_events_rate->GetYaxis()->SetTitle("Event Rate");
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        TGraph* p_graph = new TGraph(g_timegraphs_num_points, xpts, ypts);
        p_graph->SetTitle(Form("%d", vldb));
        p_graph_error_events_rate->Add(p_graph);
    }
    p_graph_error_events_rate->Draw("pmc plc");
    p_legend->Draw();
    m_objects.push_back(p_graph_error_events_rate);
}

void EventsPage::Update() { 
    DataReader* p_reader = DataReader::GetInstance();
    long int current_time = std::chrono::duration_cast
                            <std::chrono::milliseconds>
                            (std::chrono::system_clock::now()
                             .time_since_epoch()).count();

    long int last_update_dt = current_time - PageManager::GetInstance()->GetLastUpdatedTime();

    // Not pretty; see comment in ::Initialize()
    for (int i = 0; i < 8; ++i) {

        // The one case where the plot type is different
        if (i == 1) {
            TH1I* p_hist = static_cast<TH1I*>(m_objects[i]);
            for (const int& mg : p_reader->buffer_machineguns) {
                p_hist->Fill(mg);
            }

        // Everything else can be handled similarly
        } else {
            TMultiGraph* p_mg = static_cast<TMultiGraph*>(m_objects[i]);
            TList* list = p_mg->GetListOfGraphs();
            double xmin, ymin, xmax, ymax;
            for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
                TGraph* p_graph = static_cast<TGraph*>(list->At(vldb));
                p_graph->MovePoints(-(last_update_dt / 1000.), 0.);
                while (p_graph->GetPointX(0) < -g_timegraphs_window_seconds) {
                    p_graph->RemovePoint(0);
                }

                if (i == 0) {
                    p_graph->AddPoint(0, (p_reader->frame_start_time)[vldb]);
                }

                else if (i == 2) {
                    p_graph->AddPoint(0, (p_reader->num_complete_events)[vldb]);
                }
                
                else if (i == 3) {
                    int delta = (p_reader->num_complete_events)[vldb] - (m_num_complete_events_last)[vldb];
                    p_graph->AddPoint(0, delta);
                    m_num_complete_events_last[vldb] = (p_reader->num_complete_events)[vldb];
                }

                else if (i == 4) {
                    p_graph->AddPoint(0, (p_reader->num_incomplete_events)[vldb]);
                }

                else if (i == 5) {
                    int delta = (p_reader->num_incomplete_events)[vldb] - (m_num_incomplete_events_last)[vldb];
                    p_graph->AddPoint(0, delta);
                    m_num_incomplete_events_last[vldb] = (p_reader->num_incomplete_events)[vldb];
                }
                
                else if (i == 6) {
                    p_graph->AddPoint(0, (p_reader->num_bad_events)[vldb]);
                }

                else if (i == 7) {
                    int delta = (p_reader->num_bad_events)[vldb] - (m_num_bad_events_last)[vldb];
                    p_graph->AddPoint(0, delta);
                    m_num_bad_events_last[vldb] = (p_reader->num_bad_events)[vldb];
                }

                p_graph->ComputeRange(xmin, ymin, xmax, ymax);
            }

            p_mg->SetMaximum(ymax * 1.05);
        }
    }
}


void EventsPage::Clear() {
}

void EventsPage::Reset() {
    for (int vldb = 0; vldb < g_NUM_VLDB; ++vldb) {
        m_num_complete_events_last[vldb] = 0;
        m_num_incomplete_events_last[vldb] = 0;
        m_num_bad_events_last[vldb] = 0;
    }
}
