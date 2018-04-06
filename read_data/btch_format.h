#ifndef BTCH_FORMAT_H
#define BTCH_FORMAT_H

#include <algorithm>
#include <fstream>
#include <string>
#include <limits>
#include <vector>
#include <time.h>

#include "misc.h"
#include "histogram.h"
#include "edge.h"

namespace katch {

namespace btch_format {

#define NUMBER_OF_DYN_HISTOGRAMS 96
#define SECONDS_IN_DAY 86400
#define SECONDS_IN_DYN_HISTOGRAM SECONDS_IN_DAY/NUMBER_OF_DYN_HISTOGRAMS

using EdgeCost = katch::Edge;

/**
 * Class to hold file stream and manage write operations
 */
class OutputFile {
    std::ofstream _file_output_stream;
    std::string _file_name;

private:
    /**
     * Find the 15 minute interval that the tm falls into
     * @param time_start
     * @return
     */
    uint32_t calck_time_interval(struct tm time_start) const {
        auto quater_counter = floor(time_start.tm_min/15);
        auto hour_counter = time_start.tm_hour*4;  // 4 quarters in an hour
        return quater_counter + hour_counter;  // 04:17 would return 4*4 + 17/15 = 17
    }

public:
    /**
     * The only defined constructor
     * @param file_name file path
     */
    OutputFile(const std::string& file_name)
    : _file_output_stream(file_name), _file_name(file_name) {
        if ( _file_name.empty() )
            std::cerr << "Empty output file name given.\n";
        if ( !_file_output_stream.good() )
            std::cerr << "BTCH output file '" << _file_name << "' not good.\n";
    }

    /**
     * Close the file
     */
    void close() {
        if ( !_file_output_stream.is_open() ) {
            std::cerr << "Trying to close a non-open file '" << _file_name << "'.";
            return;
        }
        if ( !_file_output_stream.good() ) {
            std::cerr << "Output file '" << _file_name << "' not good.\n";
            return;
        }
        _file_output_stream.close();
    }

    // write functions
    void write_tab() {
        _file_output_stream.flush();
        _file_output_stream << "\t";
    }

    void write_char(const char *value, bool space = false) {
        _file_output_stream.flush();
        _file_output_stream << value;
        if (space)
            _file_output_stream << " ";
    }

    void write_uint32(const uint32_t value, bool space = false) {
        _file_output_stream.flush();
        _file_output_stream << value;
        if (space)
            _file_output_stream << " ";
    }

    void write_newline() {
        _file_output_stream << "\n";
    }

    void write_bucket(const std::pair<uint32_t, double> value, const bool space = false) {
        _file_output_stream.flush();
        // write cost
        _file_output_stream << value.first;
        _file_output_stream << ":";
        // write probability
        _file_output_stream << value.second;
        if (space)
            _file_output_stream << " ";
    }

    void write_histogram(const katch::Histogram histogram) {
        auto buckets = histogram.get_buckets();
        // write start bracket
        write_char("{");
        // write start time
        write_uint32(calck_time_interval(histogram.get_tp_start()), true);
        for(auto bucket : buckets) {
            write_bucket(bucket);
            if (*buckets.rbegin() != bucket) {  // is this the last bucket? If so, skip the ','
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
        // write the _constant_ start time (This is the only difference from the other write_histogram function.
                                        // They should probably be merged)
        write_uint32(0, true);
        for(auto bucket : buckets) { // buckets
            write_bucket(bucket);
            if (*buckets.rbegin() != bucket) {  // is this the last bucket? If so, skip the ','
                write_char(",", true);
            }
        }
        // write end bracket
        write_char("}");
    }

    void write_edgecost(const EdgeCost edge, const TimeType time_type) {
        // write the edge description
        write_uint32(edge.get_edge_id());   // id
        write_tab();
        write_uint32(edge.get_source());   // source
        write_tab();
        write_uint32(edge.get_target());   // target
        write_tab();

        auto histograms = edge.get_cost().get_histograms();

        if (time_type == alldata) {
            assert(histograms.size() == 1);  // One histogram for the entire week
            Histogram allday_histogram = histograms[0];
            write_histogram_alldata(allday_histogram);
            write_newline();
        }
        else if (time_type == peak) {
            assert(histograms.size() == 6);  // One histogram for every peak, + one for the weekend
            for (auto histogram : histograms) {
                write_histogram(histogram); // peaks
            }
            write_char("#", true);  // the separation character
            write_histogram(histograms[5]); // weekend
            write_newline();
        }
        else if (time_type == days) {
            assert(histograms.size() == NUMBER_OF_DYN_HISTOGRAMS*2);  // One histogram for every time interval. One set
                                                                      // for weekdays and one set for weekends
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

#endif /* BTCH_FORMAT_H */
