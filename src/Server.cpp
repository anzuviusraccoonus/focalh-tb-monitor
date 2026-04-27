#include "spdlog/spdlog.h"
#include "Server.h"
#include "ChannelMapping.h"
#include "DataReader.h"
#include "PageManager.h"
#include "pages/SpectralPage.h"
#include "globals.h"

ClassImp(Server)

Server* Server::mp_instance = nullptr;

Server::Server() : THttpServer(Form("http:%d;rw;noglobal", g_server_port)) {
    spdlog::info("Initializing Server");
    m_debug = false;
	this->SetName("Server");
	Register("/", this);


    // Don't be fooled! The space used in the command names aren't regular spaces;
    // these will cause the command to fail for some reason.
    // Instead, use this character that is enclosed in quotes --> " "

	RegisterCommand("/Control/Set Target",              "/Server/->SetTargetFile(\"%arg1%\")");
	RegisterCommand("/Control/Start Reader",            "/Server/->StartTailing()");
	RegisterCommand("/Control/Stop Reader",             "/Server/->StopTailing()");
	RegisterCommand("/Control/Clear Pages",             "/Server/->ClearPages()");
    RegisterCommand("/Control/Load Mapping",            "/Server/->LoadMapping(\"%arg1%\")");
	RegisterCommand("/Control/Print Mapping",           "/Server/->PrintMapping()");
    RegisterCommand("/Control/Set Graph Time Window",   "/Server/->SetTimegraphsWindow(%arg1%)");
    RegisterCommand("/Control/Set Page Update Interval","/Server/->SetPageUpdateInterval(%arg1%)");
    RegisterCommand("/Control/Set Spectral Graph MG",   "/Server/->SetWhichMachinegun(%arg1%)");
    RegisterCommand("/Control/Export Graphs to PNG",    "/Server/->SaveGraphs(\"%arg1%\", \".png\")");
    RegisterCommand("/Control/Export Graphs to ROOT",   "/Server/->SaveGraphs(\"%arg1%\", \".root\")");
    RegisterCommand("/Control/Toggle Debug Output",     "/Server/->ToggleDebugMode()");
}

void Server::RegisterPage(Page* p_page) {
    Register(p_page->GetPath(), p_page->GetCanvasPtr());
}

void Server::PrintMapping() {
    ChannelMapping::GetInstance()->PrintMapping();
}

void Server::StartTailing() {
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

void Server::SetPageUpdateInterval(double seconds) {
    spdlog::info("Setting update interval of pages to {} seconds", seconds);
    unsigned int milliseconds = seconds * 1000.;
    PageManager::GetInstance()->SetUpdateInterval(milliseconds);
}

void Server::SetWhichMachinegun(int mg) {
    SpectralPage* p_page = static_cast<SpectralPage*>(PageManager::GetInstance()->GetPagePtr("Spectral Graphs"));
    p_page->SetWhichMachinegun(mg);
}


void Server::LoadMapping(std::string path) {
    ChannelMapping::GetInstance()->LoadMapping(path);
}

void Server::SaveGraphs(std::string path, std::string filetype) {
    PageManager::GetInstance()->SaveGraphs(path, filetype);
}
