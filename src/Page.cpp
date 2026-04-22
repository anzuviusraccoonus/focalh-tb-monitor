#include "Page.h"
#include "spdlog/spdlog.h"

Page::Page(const char* name, const char* title, void* pData, const char* path) {
	mp_canvas = new TCanvas(name, title, 1920, 1080);
	mp_data = pData;
    m_path = path;
}

std::string Page::GetName() {
    return mp_canvas->GetName();
}

const char* Page::GetPath() {
    return m_path;
}

TCanvas* Page::GetCanvasPtr() {
    return mp_canvas;
}
