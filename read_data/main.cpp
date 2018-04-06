//
// Created by anders on 11/3/17.
// This project requires C++14 and CMake to compile
//

#include <iostream>
#include "hist_cost.h"
#include "hist_format.h"
#include "btch_format.h"

/**
 * One histogram for the entire day
 * There are 96 indices, one for each quarter of the day
 * 0 = 00:00 - 00:15
 * 95 = 23:45 - 23:59
 * */
void all_day(const std::string &cost_input_file_name) {
    std::cout << "Reading data file" << std::endl;
    auto edges_and_type = katch::hist_format::read_edges(cost_input_file_name, alldata);

    std::cout << edges_and_type.second + "_" + "ALL" << std::endl;
    katch::btch_format::OutputFile _btch_output_file(edges_and_type.second + "_" + "ALL");
    for (auto &edge : edges_and_type.first)
        _btch_output_file.write_edgecost(std::move(edge), alldata);
    _btch_output_file.close();

    std::cout << "Done writing" << std::endl;
}

/**
 * multiple histograms. 5 for weekdays, 1 for weekends
 * weekdays: mon-fri
 *  00:00 - 07:00 off
 *  07:00 - 08:30 peak
 *  08:30 - 15:00 off
 *  15:00 - 17:00 peak
 *  17:00 - 24:00 off
 * weekends: sun-sat
 *  00:00 - 24:00
*/
void peak_vs_off_peak(const std::string &cost_input_file_name) {
    std::cout << "Reading data file" << std::endl;
    auto edges_and_type = katch::hist_format::read_edges(cost_input_file_name, peak);

    std::cout << edges_and_type.second + "_" + "PEAK" << std::endl;
    katch::btch_format::OutputFile _btch_output_file(edges_and_type.second + "_" + "PEAK");
    for (auto &edge : edges_and_type.first)
        _btch_output_file.write_edgecost(std::move(edge), peak);
    _btch_output_file.close();

    std::cout << "Done writing" << std::endl;
}

/**
 * 96 histograms two times:
 * 96 for weekdays
 * 96 for weekends
 * This type will cover very single timeinterval for every single day
 */
void weekdays_vs_weekends(const std::string &cost_input_file_name) {
    std::cout << "Reading data file" << std::endl;
    auto edges_and_type  = katch::hist_format::read_edges(cost_input_file_name, days);

    std::cout << edges_and_type.second + "_" + "15M" << std::endl;
    katch::btch_format::OutputFile _btch_output_file(edges_and_type.second + "_" + "15M");
    for (auto &edge : edges_and_type.first)
        _btch_output_file.write_edgecost(std::move(edge), days);
    _btch_output_file.close();

    std::cout << "Done writing" << std::endl;
}

int main(int argc, char** argv) {
    /***
     * The first argument should be the path to the input file.
     */
    auto cost_input_file_name(argv[1]);
    all_day(cost_input_file_name);
    peak_vs_off_peak(cost_input_file_name);
    weekdays_vs_weekends(cost_input_file_name);
}
