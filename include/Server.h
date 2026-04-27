#ifndef __MONITOR_SERVER_H__
#define __MONITOR_SERVER_H__

#include <THttpServer.h>
#include "Page.h"

class Server : public THttpServer {
	public:
		static Server* GetInstance() {
			if (mp_instance == nullptr) {
				mp_instance = new Server();
			}

			return mp_instance;
		}

        void RegisterPage(Page* p_page);
		void PrintMapping();
		void StartTailing();
		void StopTailing();
		void SetTargetFile(std::string path);
		void ClearPages();
        void ToggleDebugMode();
        void SetTimegraphsWindow(double seconds);
        void SetPageUpdateInterval(double seconds);
        void SetWhichMachinegun(int mg);
        void LoadMapping(std::string path);
        void SaveGraphs(std::string path, std::string filetype);

		ClassDefOverride(Server, 1)	

	private:
		Server();
		static Server* mp_instance;
        bool m_debug;
};

#endif
