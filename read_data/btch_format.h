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
#include "misc.h"

namespace katch
{

namespace btch_format
{

#define NUMBER_OF_DYN_HISTOGRAMS 96

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_DYN_HISTOGRAM SECONDS_IN_DAY/NUMBER_OF_DYN_HISTOGRAMS

using EdgeCost = katch::Edge;

class OutputFile
{

    std::ofstream _file_output_stream;
    std::string _file_name;
    uint32_t _period;
    uint32_t _n_nodes;
    uint32_t _current_n_edges;

private:
    uint32_t calck_time_interval(struct tm time_start) {
        auto quater_counter = floor(time_start.tm_min/15);
        auto hour_counter = time_start.tm_hour*4;
        return quater_counter + hour_counter;
    }

public:
    OutputFile(const std::string& file_name, const uint32_t& n_nodes, const uint32_t& period, const uint32_t n_edges)
    : _file_output_stream(file_name),
      _file_name(file_name),
      _period( period ),
      _n_nodes( n_nodes ),
      _current_n_edges( 0 )
    {
        if ( _file_name.empty() )
            std::cerr << "Empty output file name given.\n";

        if ( !_file_output_stream.good() ) {
            std::cerr << "BTCH output file '" << _file_name << "' not good.\n";
        }
    }

    void close()
    {
        if ( ! _file_output_stream.is_open() )
        {
            std::cerr << "Trying to close a non-open BTCH file '" << _file_name << "'.";
            return;
        }

        if ( ! _file_output_stream.good() )
        {
            std::cerr << "BTCH output file '" << _file_name << "' not good.\n";
            return;
        }

        _file_output_stream.close();
    }

    // write functions
    void write_tab()
    {
        _file_output_stream.flush();
        _file_output_stream << "\t";
    }

    void write_char(const char *value, bool space = false)
    {
        _file_output_stream.flush();
        _file_output_stream << value;
        if (space)
            _file_output_stream << " ";
    }

    void write_uint32(const uint32_t value, bool space = false)
    {
        _file_output_stream.flush();
        _file_output_stream << value;
        if (space)
            _file_output_stream << " ";
    }

    void write_newline() {
        _file_output_stream << "\n";
    }

    void write_bucket(std::pair<uint32_t, double> value, bool space = false)
    {
        _file_output_stream.flush();
        // write cost
        _file_output_stream << value.first;
        _file_output_stream << ":";
        // write probability
        _file_output_stream << value.second;
        if (space)
            _file_output_stream << " ";
    }

    void write_histogram(const Histogram histogram) {
        auto buckets = histogram.get_buckets();
        // write start bracket
        write_char("{");
        // write start time
        write_uint32(calck_time_interval(histogram.get_tp_start()), true);
        for(auto bucket : buckets) { // buckets
            write_bucket(bucket);
            if (*buckets.rbegin() != bucket) {
                write_char(",", true);
            }
        }
        // write end bracket
        write_char("}", true);
    }

    void write_histogram_alldata(const Histogram histogram) {
        auto buckets = histogram.get_buckets();
        // write start bracket
        write_char("{");
        // write constant start time
        write_uint32(0, true);
        for(auto bucket : buckets) { // buckets
            write_bucket(bucket);
            if (*buckets.rbegin() != bucket) {
                write_char(",", true);
            }
        }
        // write end bracket
        write_char("}");
    }

    void write_edgecost(EdgeCost edge, TimeType time_type)
    {
        // write the edge description
        write_uint32(edge.get_edge_id());   // id
        write_tab();
        write_uint32(edge.get_source());   // source
        write_tab();
        write_uint32(edge.get_target());   // target
        write_tab();

        auto histograms = edge.get_cost().get_histograms();

        if (time_type == alldata) {
            assert(histograms.size() == 1);
            Histogram allday_histogram = histograms[0];
            write_histogram_alldata(allday_histogram);
            write_newline();
        }
        else if (time_type == peak) {
            assert(histograms.size() == 6);
            for (auto histogram : histograms) {
                write_histogram(histogram); // peaks
            }
            write_char("#", true);  // the separation character
            write_histogram(histograms[5]); // weekend
            write_newline();
        }
        else if (time_type == days) {
            assert(histograms.size() == NUMBER_OF_DYN_HISTOGRAMS*2);
            auto const half_size = histograms.size() / 2;
            std::vector<Histogram > weekdays(histograms.begin(), histograms.begin() + half_size);
            std::vector<Histogram > weekends(histograms.begin() + half_size, histograms.end());
            for (auto hist : weekdays)
                write_histogram(hist);
            write_char("#", true);  // the separation character
            for (auto hist : weekends)
                write_histogram(hist);
            write_newline();
        }
        else {
            std::cout << "Invalid time type selected" << std::endl;
        }
    }
};
}
}

#endif /* KATCH_BTCH_FORMAT_H_ */
