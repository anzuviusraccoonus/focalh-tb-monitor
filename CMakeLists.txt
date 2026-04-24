cmake_minimum_required(VERSION 3.20)
project(FocalHMonitor VERSION 1.1.1 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ROOT_IGNORE_PACKAGE_VGT TRUE)
set(CMAKE_BUILD_TYPE Debug)

find_package(ROOT REQUIRED COMPONENTS
	Core RIO Hist Tree TreePlayer MathCore Graf Gpad Rint Thread
)

find_package(Threads REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.14.1
)

set(SPDLOG_BUILD_SHARED ON)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(external/binparse)
include_directories(external/binparse/include/binparse)

include_directories(include)
ROOT_GENERATE_DICTIONARY(ServerDict Server.h MODULE Monitor LINKDEF LinkDef.h)
add_library(server SHARED src/Server.cpp ServerDict.cxx)
target_include_directories(server PRIVATE include ${ROOT_INCLUDE_DIRS})
target_link_libraries(server PRIVATE spdlog::spdlog)

add_executable(Monitor main.cpp)
target_include_directories(Monitor PRIVATE include ${ROOT_INCLUDE_DIRS})
target_sources(Monitor 
		PRIVATE 
		src/globals.cpp
		src/Server.cpp
		src/DataReader.cpp
		src/PageManager.cpp
        src/ChannelMapping.cpp
		src/Page.cpp 
		src/pages/StatusPage.cpp
		src/pages/ChannelsPage.cpp
        src/pages/SpectralPage.cpp
		src/pages/HeatmapPage.cpp
        src/pages/TSPage1.cpp
        src/pages/TSPage2.cpp
        src/pages/SummaryPage.cpp
)

target_link_libraries(Monitor
    PRIVATE
    spdlog::spdlog
    binparse::binparse
    ROOT::Core ROOT::RIO ROOT::Hist ROOT::Tree ROOT::MathCore ROOT::RHTTP ROOT::Gpad
    Threads::Threads
	${ROOT_LIBRARIES}
	server
)

target_compile_features(Monitor PRIVATE cxx_std_23)
