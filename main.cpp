#include <THttpServer.h>
#include <TROOT.h>

#include "external/args/args.hxx"
#include "spdlog/spdlog.h"
#include "src/PageBuilder.cpp"

#include "Server.h"
#include "PageManager.h"
#include "DataReader.h"
#include "ChannelMapping.h"
#include "globals.h"

int main(int argc, char **argv) {
    args::ArgumentParser    parser  ("Online monitoring tool for the FoCal-H prototype.");
    args::HelpFlag          help    (parser, "help", "Display this help menu and exit", {'h', "help"});

    args::Flag              arg_verbose  (parser, "verbose", 
                                         "Enable verbose/debug output",
                                         {'v'});

    args::ValueFlag<int>    arg_update   (parser, "time",
                                         "Update interval of the pages, in milliseconds",
                                         {'t'}, 1000);

    try {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help) {
        std::cout << parser;
        return 0;
    }
    
    if (arg_verbose) {
        spdlog::set_level(spdlog::level::debug);
	}

    // This should also be instantiation of the singletons
	Server*         p_server        = Server::GetInstance();
    PageManager*    p_pagemanager   = PageManager::GetInstance();
    DataReader*     p_reader        = DataReader::GetInstance();
    ChannelMapping* p_mapping       = ChannelMapping::GetInstance();
    
    BuildPages(); // Page setup, layout, etc. happens in this function

	// Load default channel mapping
    p_mapping->LoadMapping("./channelmapping");

	p_server->SetItemField("/", "_monitoring", "1000");
	p_server->SetTimer(100, false); // false = run asynchronously

	p_pagemanager->SetUpdateInterval(arg_update.Get());
	
	while (true) {
        // This is to keep the server alive
        // Could probably be done in a better way, but oh well..
	}

	return 0;
}
