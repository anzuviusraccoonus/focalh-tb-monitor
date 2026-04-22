#include <chrono>
#include "spdlog/spdlog.h"
#include "PageManager.h"
#include "Server.h"
#include "globals.h"

PageManager* PageManager::mp_instance = nullptr;

PageManager::PageManager() {
    spdlog::info("Initializing PageManager");
    m_pages = {};
	m_update_interval = 1000;	
	std::thread update_thread(&PageManager::UpdatePages, this);
	update_thread.detach();
}

PageManager::~PageManager() {
}

void PageManager::AddPage(Page* p_page) {
    spdlog::debug("Adding page {} to browser", p_page->GetName());

    m_pages.push_back(p_page);
	p_page->Initialize();

	Server* p_server = Server::GetInstance();
    p_server->RegisterPage(p_page);
}

void PageManager::UpdatePages() {
    spdlog::debug("Updating pages");
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(m_update_interval));
		for (Page* p : m_pages) {
	        std::scoped_lock lock(g_mutex);
            spdlog::debug("Updating {}", p->GetName());
			p->Update();
		}
	}
}

void PageManager::ClearPages() {
	spdlog::info("Clearing pages");
	for (Page* p : m_pages) {
		std::scoped_lock lock(g_mutex);
		spdlog::debug("Clearing {}", p->GetName());
		p->Clear();
	}
}

void PageManager::ResetPages() {
	spdlog::info("Resetting pages");
	for (Page* p : m_pages) {
		std::scoped_lock lock(g_mutex);
		spdlog::debug("Resetting {}", p->GetName());
		p->Reset();
	}
}

void PageManager::SetUpdateInterval(unsigned int milliseconds) {
	m_update_interval = milliseconds;
}

unsigned int PageManager::GetUpdateInterval() {
	return m_update_interval;
}
