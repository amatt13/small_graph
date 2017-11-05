/*
 * katch/io/btch_format.h
 *
 *
 * This file is part of
 *
 * KaTCH -- Karlsruhe Time-Dependent Contraction Hierarchies
 *
 * Copyright (C) 2015
 *
 * Institut fuer Theroretische Informatik,
 * Karlsruher Institut fuer Technology (KIT),
 * 76131 Karlsruhe, Germany
 *
 * Author: Gernot Veit Batz
 *
 *
 *
 * KaTCH is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * KaTCH is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with Contraction Hierarchies; see the file COPYING;
 * if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KATCH_BTCH_FORMAT_H_
#define KATCH_BTCH_FORMAT_H_

#include <algorithm>
#include <fstream>
#include <string>
#include <limits>
#include <vector>
#include <time.h>

#include "basic.h"
#include "binary.h"
#include "misc.h"

namespace katch
{

namespace btch_format
{

constexpr uint32_t FILE_SEPARATOR = 0x00772255;
constexpr uint32_t FILE_TERMINATOR = 0xaabbccdd;
constexpr uint32_t UINT32_MAX_VALUE = std::numeric_limits<uint32_t>::max();
using EdgeCost = katch::Edge;

class OutputFile
{

    std::ofstream _file_output_stream;
    std::string _file_name;
    uint32_t _period;
    uint32_t _n_nodes;
    uint32_t _current_n_edges;

public:
    OutputFile(const std::string& file_name, const uint32_t& n_nodes, const uint32_t& period, const uint32_t n_edges)
    : _file_output_stream(file_name),
      _file_name(file_name),
      _period( period ),
      _n_nodes( n_nodes ),
      _current_n_edges( 0 )
    {
        if ( _file_name.empty() )
            KATCH_ERROR("Empty output file name given.\n");

        if ( _file_output_stream.good() )
        {
            // write version number
            write_uint32(1);

            // write number of nodes
            write_uint32(n_nodes);

            // write a place holder for the number of edges
            write_uint32(n_edges);

            // write period of TTFs
            write_uint32(period);

            int j = 0;
            // write a place holder for the level info
            for ( uint32_t node_id = 0; node_id < n_nodes; ++node_id ) {
                write_uint32(UINT32_MAX_VALUE);
                j++;
            }

            // write separator between level info and edge info
            write_uint32(FILE_SEPARATOR);
        }
        else
        {
            KATCH_ERROR("BTCH output file '" << _file_name << "' not good.\n");
        }
    }

    OutputFile(const std::string& file_name) : _file_name(file_name) {}

    ~OutputFile()
    {
        if ( _file_output_stream.is_open() )
        {
            KATCH_WARNING("Automatically closing non-closed BTCH output file on destruction.\n");
            close();
        }
    }

    void open_and_read(std::string input_file_name)
    {
        if (input_file_name == "")
            KATCH_ERROR("Empty input file name given.\n");


        KATCH_STATUS("Reading BTCH file '" << input_file_name << "'...");

        std::ifstream input_btch_file(input_file_name);

        if ( ! input_btch_file.is_open() )
        {
            KATCH_CONTINUE_STATUS(" ABORT\n");
            KATCH_ERROR("Unable to open file: already open\n");
        }
    }

    void close()
    {
        //TODO something is funky
        if ( ! _file_output_stream.is_open() )
        {
            KATCH_ERROR("Trying to close a non-open BTCH file '" << _file_name << "'.");
            return;
        }

        if ( ! _file_output_stream.good() )
        {
            KATCH_ERROR("BTCH output file '" << _file_name << "' not good.\n");
            return;
        }

        // append file terminator
        write_uint32(FILE_TERMINATOR);

        // store number of edges
        _file_output_stream.seekp( 6 + 2 * sizeof(uint32_t) );
        write_uint32(_current_n_edges);

        _file_output_stream.close();
    }

    // write functions
    void write_char(const char *value)
    {
        _file_output_stream.flush();
        _file_output_stream << value;
        _file_output_stream << " ";
    }

    void write_uint32(const uint32_t value)
    {
        _file_output_stream.flush();
        _file_output_stream << value;
        _file_output_stream << " ";
    }

    void write_double(const double value)
    {
        _file_output_stream << value;
        _file_output_stream << " ";
    }

    void write_bool(const bool value)
    {
        _file_output_stream << value;
        _file_output_stream << " ";
    }

    void write_histogram(const Histogram histogram) {
        write_uint32(util::get_total_seconds(histogram.get_tp_start())); // start
        write_uint32(util::get_total_seconds(histogram.get_tp_end())); // end
        write_uint32(histogram.get_n_measurements()); // n_mes
        write_bool(histogram.is_constant()); // is_constant
        write_uint32(histogram.get_bucket_size()); // bucket_size
        for(auto bucket : histogram.get_buckets()) { // buckets
            write_char("{");
            write_uint32(bucket.first); // cost
            write_double(bucket.second); // prob
            write_char("}");
        }
    }

    void write_histcost(const HistCost histCost){
        write_uint32(histCost.get_upper()); // upper
        write_uint32(histCost.get_lower()); // lower
        write_uint32(histCost.get_histograms().size()); // number of histograms
        for (auto const histogram : histCost.get_histograms()) {
            write_histogram(histogram);  // histograms
        }
    }
    void write_edgecost(EdgeCost edge, GraphType graph_type, TimeType time_type)
    {
        write_char("#");
        write_uint32(edge.get_edge_id());   // id
        write_uint32(edge.get_source());   // source
        write_uint32(edge.get_target());   // target

        auto histograms = edge.get_cost().get_histograms();

        if (time_type == alldata) {
            assert(histograms.size() == 1);
            Histogram allday_histogram = histograms[0];
            write_histogram(allday_histogram);
            write_char("\n");
        }
        else if (time_type == peak) {

        }
        else if (time_type == days) {

        }
        else {
            std::cout << "Invalid time type selected" << std::endl;
        }
    }
};



}
}

#endif /* KATCH_BTCH_FORMAT_H_ */
