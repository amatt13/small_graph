//
// Created by anders on 11/3/17.
//

#include <iostream>
#include "hist_cost.h"
#include "edge_info.h"
#include "btch_format.h"
#include "misc.h"

/**
 * Der er 96 indexes
 * 0 = 00:00 - 00:15
 * 95 = 23:45 - 23:59.59999999999.....
 * */

using EdgeCost = katch::Edge;

// One histogram for the entire day
void allday(const std::string &cost_input_file_name, GraphType graph_type) {
    std::cout << "Reading data file" << std::endl;
    auto edges = katch::hist_format::read_edges(cost_input_file_name, alldata);

    std::cout << "Writing ''allday'' with type: " << graph_type << std::endl;
    katch::btch_format::OutputFile _btch_output_file("/home/anders/Desktop/some_file", (uint32_t)2, (uint32_t)33, 1);
    for (auto &edge : edges)
        _btch_output_file.write_edgecost(std::move(edge), graph_type, alldata);
    _btch_output_file.close();

    std::cout << "Done writing" << std::endl;
}


// multiple(6 or 5?) histograms for both type of days
void peek_vs_offpeak() {
    //code
}


// 96 histograms for each of the type of days
void weekdays_vs_weekends() {
    //code
}




int main(int argc, char** argv) {
    auto cost_input_file_name(argv[1]);
    auto current_type = aalborg;// find
    allday(cost_input_file_name, current_type);
    return 0;
}
