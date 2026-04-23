# focalh-tb-monitor
An online monitoring tool for the ALICE FoCal-H Prototype 3 read-out data. Reads the produced detector output and provides (near) instant, rudimentary analysis to serve as first-line QA.

## Installation

Clone this repo, then run cmake and make:
```
git clone https://github.com/anzuviusraccoonus/focalh-tb-monitor
cd focalh-tb-monitor
cmake -B build -S .
make -C build
```
after which the compiled binary is available in ./build/bin/

## Running

Simply run the compiled binary:
```
./build/bin/Monitor
```

Running the monitor starts a ROOT online server / object browser on the current machine, by default on port 12345. The port is defined and can be changed by modifying ```src/globals.cpp```. After modifying, re-run ```make```. Other compile-time settings are also available in this file, along with ```include/globals.h```.

After starting the server, various controls are available from the ROOT browser's interface, under the ```Control``` folder. Various *pages* are available to browse from the object list which present different quality assurance metrics once a file is selected.

## Pages

The monitor is divided into various pages, which can be loaded from the sidebar. Currently, the following pages are available:

### Overview Page

Contains information about the current target file and the data that has been read in so far. Also whether the reader is currently running or not, and timestamps showing when the last trigger and data lines were read. There is some simple statistics about the types of lines that were read.

### TS Page 1

Contains graphs relating to the rates of events and triggers that are being read in, in addition to the timestamps of events (at the time of page update) and a histogram showing how many machinegun triggers are seen for each recorded event. The points on the timestamps graph are measured every time the graph updates, and as such do not show *every* recorded event's timestamp. 

### TS Page 2

Contains graphs relating to the total (in current file) accumulated events and triggers that are being read in, as well as the total amount of data read, and a histogram showing the difference between bunch crossing counters of each sequential line.

### Spectral Graphs

This page contains two-dimensional histograms ("spectral graphs") that show recorded ADC, TOT and TOA values for each channel, grouped by each available VLDB link. The TOT and TOA values shown in these graphs are **NOT** decoded, so their values do not correspond directly to actual TOT and TOA values.

### Heatmaps

In the folder "Heatmaps", there are three pages available, showing respectively the ADC, TOT and TOA values in each channel, grouped spatially according to the loaded channel mapping, as seen from the back of the detector. If no channel mapping is loaded, or for every channel read in the data that hasn't been assigned a row/column pair in the mapping, the data will be empty. In other words, only mapped channels will be shown here and the rest are discarded.

### Waveform Plots

In the "VLDB #" folders and their subfolders, pages can be found that show ADC, TOT and TOA values of each machinegun trigger for each individual channel. These pages show *all* 38 read-out channels, even if some of these do not correspond to any physical channels. Namely, two of them will be respectively the common mode channel and calibration channel of each half-ASIC. 

### ADC Summary Page

Currently, shows just one graph with the per-event ADC sums. Only complete events, i.e. events where all machine gun triggers were present, are included.

## Controls

The following controls are available to any user with access to the server, whether locally or remotely.

##### Set Target

Sets the file to be monitored. The path entered is relative to the current working directory (i.e., from where the executable was run) and must be the full filename. Upon entering the path, the monitor will automatically start reading the file if it exists. If it does not exist (or can't be opened for some reason) an error will be logged. If the file is created *after* entering the path, the reader can be started manually with the ```Start Reader``` command.

##### Start Reader

Starts the file reader, if it is not already started. Starting the reader will cause it to read the file *from the beginning* even if the current file has not changed, e.g. if one manually stopped the reader with the ```Stop Reader``` command.

##### Stop Reader

Stops the file reader. Pages will keep updating, though no more data will be read in. Starting the reader again will cause it to start from the beginning, so this currently cannot be used as a "pause" command.

##### Clear Pages

Clears certain graphs and statistics, e.g. heatmaps and accumulated statistics. These will continue to be updated / filled with new data from the current point in the file. This can be used while the reader is running, and is also called automatically when ```Set Target``` is called.

##### Load Mapping

Loads a new channel mapping from the given file. As with ```Set Target```, the path is relative to the current working directory. This command is automatically called when the server is first started, using ```./channelmapping``` as the argument. Loading a new channel mapping will NOT currently re-do already plotted data that relies on the mapping, so it is recommended to run ```Clear Pages``` first.

##### Print Mapping

Prints the currently loaded channel mapping to the terminal. Channels that are not associated with any row/column pair are listed as "N.A.".

##### Set Graph Time Window

Some graphs show some statistic in a period of time, from current time and back some amount of time. This command sets the length of the window, i.e. how far back in time these graphs are drawn. By default, this is set to 300 seconds. When updating this to a value that is *greater* than the current window length, the graphs will grow until they show the full window. When updating this to a value that is *lesser* than the current window length, the graphs will instantly shrink to match the chosen window.

##### Export Graphs

Saves all graphs to the specified path, either as .png images or as ROOT files depending on the chosen command. If the path is ".", they will be saved to the current directory. The specified path can be either absolute or relative, and any folders that don't already exist will be created. The graphs are grouped into their respective pages, as shown on the monitor. Names of individual images/ROOT files cannot be chosen and so will override the previously saved graphs if the same folder is used.

##### Toggle Debug Output

Switches the logging level of spdlog between "debug" and "critical" (the normal setting); equivalent to running the monitor with "-v". This produces ***a lot*** of output to the terminal, and will slow down the server. In addition to mostly logging function calls, it also prints all read data to the terminal.

## Channel Mapping 

The channel mapping is, by default when the server is first started, attempted loaded from ```./channelmapping```. This file determines which channels (from the available VLDB links, ASICs and ASIC "halves") correspond to which physical row and column on the detector. The channel mapping file must have a header as ```ROW  COL  VLDB  ASIC  HALF  CHANNEL``` and each following line defines such a map. For example:
```
ROW  COL  VLDB  ASIC  HALF  CHANNEL
0  0  0  0  0  3
0  1  0  0  0  7
0  2  0  0  0  12
0  3  0  0  0  16
```

would set channels 3, 7, 12 and 16 of the first half of the first ASIC of the first VLDB link to column 0, 1, 2 and 3 of row 0, respectively. The order of the maps do not matter, except that the header must be the first line. It is also not required that every channel is mapped; such would be the case with e.g. the common mode and calibration channels (unless one would want these to be mapped, for some reason)
