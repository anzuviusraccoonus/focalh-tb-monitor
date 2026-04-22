#include "spdlog/spdlog.h"
#include "Server.h"
#include "ChannelMapping.h"
#include "DataReader.h"
#include "PageManager.h"
#include "globals.h"

ClassImp(Server)

Server* Server::mp_instance = nullptr;

Server::Server() : THttpServer(Form("http:%d;rw;noglobal", g_server_port)) {
    spdlog::info("Initializing Server");
    m_debug = false;
	this->SetName("Server");
	Register("/", this);
	RegisterCommand("/Control/Set_Target",              "/Server/->SetTargetFile(\"%arg1%\")");
	RegisterCommand("/Control/Start_Reader",            "/Server/->StartTailing()");
	RegisterCommand("/Control/Stop_Reader",             "/Server/->StopTailing()");
	RegisterCommand("/Control/Clear_Pages",             "/Server/->ClearPages()");
    RegisterCommand("/Control/Load_Mapping",            "/Server/->LoadMapping(%arg1%)");
	RegisterCommand("/Control/Print_Mapping",           "/Server/->PrintMapping()");
    RegisterCommand("/Control/Set_Graph_Time_Window",   "/Server/->SetTimegraphsWindow(%arg1%)");
    RegisterCommand("/Control/Toggle_Debug_Output",     "/Server/->ToggleDebugMode()");
}

void Server::RegisterPage(Page* p_page) {
    Register(p_page->GetPath(), p_page->GetCanvasPtr());
}

void Server::PrintMapping() {
    ChannelMapping::GetInstance()->PrintMapping();
}

void Server::StartTailing() {
    if (DataReader::GetInstance()->IsRunning()) {
        spdlog::error("Reader is already running - stop the reader first");
        return;
    }

    DataReader::GetInstance()->Start();
}	

void Server::StopTailing() {
    DataReader::GetInstance()->Stop();
}	

void Server::SetTargetFile(std::string path) {
	DataReader* p_reader = DataReader::GetInstance();
	p_reader->Stop();
	p_reader->SetTarget(path);
	p_reader->Start();	
	
    PageManager* p_pagemanager = PageManager::GetInstance();
    p_pagemanager->ClearPages();
    p_pagemanager->ResetPages();
}

void Server::ClearPages() {
    PageManager::GetInstance()->ClearPages();
}

void Server::ToggleDebugMode() {
    m_debug = not m_debug;
    if (m_debug) {
        spdlog::info("Debug mode enabled");
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::info("Debug mode disabled");
        spdlog::set_level(spdlog::level::critical);
    }
}

void Server::SetTimegraphsWindow(double seconds) {
    spdlog::info("Setting time window of graphs to {} seconds", seconds);
    g_timegraphs_window_seconds = seconds;
    g_timegraphs_num_points = (int)seconds;
}

void Server::LoadMapping(std::string path) {
    ChannelMapping::GetInstance()->LoadMapping(path);
}
