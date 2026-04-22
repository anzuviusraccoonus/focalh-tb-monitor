#ifndef __MONITOR_PAGE_H__
#define __MONITOR_PAGE_H__

#include <TCanvas.h>
#include <TObject.h>

class Page {
	public:
		Page(const char* name, const char* title, void* p_data, const char* path);
		~Page() {};

		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void Clear() = 0;
		virtual void Reset() = 0;

		std::string GetName();
        const char* GetPath();
		TCanvas* GetCanvasPtr();

	protected:
		TCanvas* mp_canvas;
		std::vector<TObject*> m_objects;
		void* mp_data;
        const char* m_path;
};

#endif
