#include <chrono>
#include <format>
#include <TPaveText.h>
#include "spdlog/spdlog.h"
#include "DataStructs.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

#include "pages/StatusPage.h"

void StatusPage::Initialize() {
	TPaveText* status = new TPaveText(0.05, 0.1, 0.95, 0.9);
	TText* line;

	const int FONTSIZE_REGULAR = 42;
    const int FONTSIZE_SMALL = FONTSIZE_REGULAR - 9;

	line = status->AddText(0.0, 0.95, "Current file:");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_REGULAR);
	
	mp_file = status->AddText(0.32, 0.95, "");
	mp_file->SetTextAlign(13);
	mp_file->SetTextSize(FONTSIZE_REGULAR);
	

	line = status->AddText(0.0, 0.85, "Reader status:");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_REGULAR);
	
	mp_reader_running = status->AddText(0.32, 0.85, "");
	mp_reader_running->SetTextAlign(13);
	mp_reader_running->SetTextSize(FONTSIZE_REGULAR);


	line = status->AddText(0.0, 0.63, "Total data read:");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_REGULAR);
	
	mp_data_read = status->AddText(0.32, 0.63, "");
	mp_data_read->SetTextAlign(13);
	mp_data_read->SetTextSize(FONTSIZE_REGULAR);
	
	
    line = status->AddText(0.07, 0.55, "Trigger lines");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_trig_lines_read = status->AddText(0.32, 0.55, "");
	mp_trig_lines_read->SetTextAlign(13);
	mp_trig_lines_read->SetTextSize(FONTSIZE_SMALL);
	
	
	line = status->AddText(0.07, 0.50, "Data lines");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_data_lines_read = status->AddText(0.32, 0.50, "");
	mp_data_lines_read->SetTextAlign(13);
	mp_data_lines_read->SetTextSize(FONTSIZE_SMALL);


	line = status->AddText(0.07, 0.45, "L0 lines");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_l0_lines_read = status->AddText(0.32, 0.45, "");
	mp_l0_lines_read->SetTextAlign(13);
	mp_l0_lines_read->SetTextSize(FONTSIZE_SMALL);
	
	
	line = status->AddText(0.07, 0.40, "L1 lines");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_l1_lines_read = status->AddText(0.32, 0.40, "");
	mp_l1_lines_read->SetTextAlign(13);
	mp_l1_lines_read->SetTextSize(FONTSIZE_SMALL);
	
	
	line = status->AddText(0.07, 0.35, "Hb packets");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_num_heartbeat_packets = status->AddText(0.32, 0.35, "");
	mp_num_heartbeat_packets->SetTextAlign(13);
	mp_num_heartbeat_packets->SetTextSize(FONTSIZE_SMALL);
	
	
	line = status->AddText(0.07, 0.30, "Sync packets");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_num_sync_packets = status->AddText(0.32, 0.30, "");
	mp_num_sync_packets->SetTextAlign(13);
	mp_num_sync_packets->SetTextSize(FONTSIZE_SMALL);
    
    
    line = status->AddText(0.44, 0.55, "( latest read @");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_last_trig_time = status->AddText(0.61, 0.55, "never )");
	mp_last_trig_time->SetTextAlign(13);
	mp_last_trig_time->SetTextSize(FONTSIZE_SMALL);
	
	
	line = status->AddText(0.44, 0.50, "( latest read @");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_last_data_time = status->AddText(0.61, 0.50, "never )");
	mp_last_data_time->SetTextAlign(13);
	mp_last_data_time->SetTextSize(FONTSIZE_SMALL);
	
    
    line = status->AddText(0.25, 0.15, "Server last updated at ");
	line->SetTextAlign(13);
	line->SetTextSize(FONTSIZE_SMALL);
	
	mp_last_server_update_time = status->AddText(0.52, 0.15, "never");
	mp_last_server_update_time->SetTextAlign(13);
	mp_last_server_update_time->SetTextSize(FONTSIZE_SMALL);
		
	status->Draw();
}

void StatusPage::Update() {
	DataReader* p_reader = DataReader::GetInstance();

    auto time = std::chrono::system_clock::now();
    std::string s = std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::floor<std::chrono::seconds>(time));
    mp_last_server_update_time->SetText(mp_last_server_update_time->GetX(), 
                                        mp_last_server_update_time->GetY(), 
                                        s.c_str());

	mp_file->SetText(mp_file->GetX(), 
                     mp_file->GetY(),
  	 			     p_reader->GetTarget().c_str());

	if (p_reader->IsRunning()) {
		mp_reader_running->SetText(mp_reader_running->GetX(), mp_reader_running->GetY(), "Running");
	} else {
		mp_reader_running->SetText(mp_reader_running->GetX(), mp_reader_running->GetY(), "Stopped");
	}

    if (p_reader->trig_lines_read > m_num_trig_lines_read) {
	    std::string s = std::format("{:%Y-%m-%d %H:%M:%S} )", std::chrono::floor<std::chrono::seconds>(time));
	    mp_last_trig_time->SetText(mp_last_trig_time->GetX(), mp_last_trig_time->GetY(), s.c_str());
    }
    
    if (p_reader->data_lines_read > m_num_data_lines_read) {
	    std::string s = std::format("{:%Y-%m-%d %H:%M:%S} )", std::chrono::floor<std::chrono::seconds>(time));
	    mp_last_data_time->SetText(mp_last_data_time->GetX(), mp_last_data_time->GetY(), s.c_str());
    }

    m_num_trig_lines_read = p_reader->trig_lines_read;
	mp_trig_lines_read->SetText(mp_trig_lines_read->GetX(), mp_trig_lines_read->GetY(),
							    std::to_string(p_reader->trig_lines_read).c_str());

    m_num_data_lines_read = p_reader->data_lines_read;
	mp_data_lines_read->SetText(mp_data_lines_read->GetX(), mp_data_lines_read->GetY(),
							    std::to_string(p_reader->data_lines_read).c_str());

	mp_l0_lines_read->SetText(mp_l0_lines_read->GetX(), mp_l0_lines_read->GetY(),
							  std::to_string(p_reader->l0_lines_read).c_str());

	mp_l1_lines_read->SetText(mp_l1_lines_read->GetX(), mp_l1_lines_read->GetY(),
							  std::to_string(p_reader->l1_lines_read).c_str());

	mp_num_heartbeat_packets->SetText(mp_num_heartbeat_packets->GetX(), mp_num_heartbeat_packets->GetY(),
							          std::to_string(p_reader->num_heartbeat_packets).c_str());

	mp_num_sync_packets->SetText(mp_num_sync_packets->GetX(), mp_num_sync_packets->GetY(),
						    	 std::to_string(p_reader->num_sync_packets).c_str());

	static const char *SIZES[] = {"B", "kB", "MB", "GB"};
	size_t div = 0;
	size_t rem = 0;
	size_t size = p_reader->bytes_read;
	while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES) - 1) {
		rem = (size % 1024);
		++div;
		size /= 1024;
	}

	s = std::format("{} {}", (int)((float)size + (float)rem / 1024.0), SIZES[div]);
	mp_data_read->SetText(mp_data_read->GetX(), mp_data_read->GetY(), s.c_str());
}

void StatusPage::Clear() {
}

void StatusPage::Reset() {
	mp_data_read->SetText(mp_data_read->GetX(), mp_data_read->GetY(), "0 B");
	mp_trig_lines_read->SetText(mp_trig_lines_read->GetX(), mp_trig_lines_read->GetY(), "0");
	mp_data_lines_read->SetText(mp_data_lines_read->GetX(), mp_data_lines_read->GetY(), "0");
	mp_l0_lines_read->SetText(mp_l0_lines_read->GetX(), mp_l0_lines_read->GetY(), "0");
	mp_l1_lines_read->SetText(mp_l1_lines_read->GetX(), mp_l1_lines_read->GetY(), "0");
	mp_num_heartbeat_packets->SetText(mp_num_heartbeat_packets->GetX(), mp_num_heartbeat_packets->GetY(), "0");
	mp_num_sync_packets->SetText(mp_num_sync_packets->GetX(), mp_num_sync_packets->GetY(), "0");
	mp_last_data_time->SetText(mp_last_data_time->GetX(), mp_last_data_time->GetY(), "never )");
	mp_last_trig_time->SetText(mp_last_trig_time->GetX(), mp_last_trig_time->GetY(), "never )");
    m_num_trig_lines_read = 0;
    m_num_data_lines_read = 0;
}
