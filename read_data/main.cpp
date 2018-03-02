//
// Created by anders on 11/3/17.
// This project requires C++14 and CMake to compile
//

#include <iostream>
#include "hist_cost.h"
#include "hist_format.h"
#include "btch_format.h"

/**
 * Der er 96 indexes
 * 0 = 00:00 - 00:15
 * 95 = 23:45 - 23:59.59999999999.....
 * */

// One histogram for the entire day
void allday(const std::string &cost_input_file_name) {
    std::cout << "Reading data file" << std::endl;
    auto edges_and_type = katch::hist_format::read_edges(cost_input_file_name, alldata);

    std::cout << edges_and_type.second + "_" + "ALL" << std::endl;
    katch::btch_format::OutputFile _btch_output_file(edges_and_type.second + "_" + "ALL", (uint32_t)2, (uint32_t)33, 1);
    for (auto &edge : edges_and_type.first)
        _btch_output_file.write_edgecost(std::move(edge), alldata);
    _btch_output_file.close();

    std::cout << "Done writing" << std::endl;
}

/* multiple histograms. 5 for weekdays, 1 for weekends
 * weekdays: mon-fri
 *  00:00 - 07:00 off
 *  07:00 - 08:30 peak
 *  08:30 - 15:00 off
 *  15:00 - 17:00 peak
 *  17:00 - 24:00 off
 * weekends: sun-sat
 *  00:00 - 24:00
*/
void peak_vs_offpeak(const std::string &cost_input_file_name) {
    std::cout << "Reading data file" << std::endl;
    auto edges_and_type = katch::hist_format::read_edges(cost_input_file_name, peak);

    std::cout << edges_and_type.second + "_" + "PEAK" << std::endl;
    katch::btch_format::OutputFile _btch_output_file(edges_and_type.second + "_" + "PEAK", (uint32_t)2, (uint32_t)33, 1);
    for (auto &edge : edges_and_type.first)
        _btch_output_file.write_edgecost(std::move(edge), peak);
    _btch_output_file.close();

    std::cout << "Done writing" << std::endl;
}


// 96 histograms for each of the type of days
void weekdays_vs_weekends(const std::string &cost_input_file_name) {
    std::cout << "Reading data file" << std::endl;
    auto edges_and_type  = katch::hist_format::read_edges(cost_input_file_name, days);

    std::cout << edges_and_type.second + "_" + "15M" << std::endl;
    katch::btch_format::OutputFile _btch_output_file(edges_and_type.second + "_" + "15M", (uint32_t)2, (uint32_t)33, 1);
    for (auto &edge : edges_and_type.first)
        _btch_output_file.write_edgecost(std::move(edge), days);
    _btch_output_file.close();

    std::cout << "Done writing" << std::endl;
}


int main(int argc, char** argv) {
    // the data file to read
    // as of now, we are reading the entirety of the input file three times (one time for each graph type). This could be changed such that only one read would be necessary
    auto cost_input_file_name(argv[1]);
    //allday(cost_input_file_name);
    //peak_vs_offpeak(cost_input_file_name);
    weekdays_vs_weekends(cost_input_file_name);
}
