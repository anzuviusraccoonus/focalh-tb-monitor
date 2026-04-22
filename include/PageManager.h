#ifndef __MONITOR_PAGEMANAGER_H__
#define __MONITOR_PAGEMANAGER_H__

#include <TPaveText.h>
#include <TText.h>
#include "Page.h"

class PageManager {
	public:
        static PageManager* GetInstance() {
            if (mp_instance == nullptr) {
                mp_instance = new PageManager();
            }

            return mp_instance;
        }

		void AddPage(Page* pPage);
		void UpdatePages();
		void ClearPages();
        void ResetPages();
		void SetUpdateInterval(unsigned int milliseconds);
		unsigned int GetUpdateInterval();

	private:
        PageManager();
        ~PageManager();
        static PageManager* mp_instance;

		std::vector<Page*> m_pages;
		unsigned int m_update_interval;
};

#endif
