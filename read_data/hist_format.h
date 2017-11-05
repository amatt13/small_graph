#ifndef KATCH_HIST_FORMAT_H_
#define KATCH_HIST_FORMAT_H_

#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <time.h>
#include <stdlib.h>

#include "edge_info.h"
#include "histogram.h"
#include "edge.h"
#include "misc.h"
#include "boost/date_time/posix_time/posix_time.hpp"

#define KATCH_NUMBER_OF_DYN_HISTOGRAMS 96

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_DYN_HISTOGRAM SECONDS_IN_DAY/KATCH_NUMBER_OF_DYN_HISTOGRAMS

namespace katch
{
    struct measurement{
        uint32_t type;
        uint32_t traversal_time;
        std::tm tm;
    };

	namespace hist_format
	{
		using namespace boost::posix_time;
		std::vector<measurement> ReadMeasurements(std::ifstream& stream, bool &reached_end)
		{
			std::vector<measurement> gps_measurements;
			if(stream.peek() == '#'){
				std::string line;
				std::getline(stream, line);
				KATCH_STATE("TWO # AFTER EACHOTHER." << line << "\n");
				reached_end = true;
				gps_measurements.clear();
				return gps_measurements;
			}

			Histogram result;
			char buffer[10];

			if(stream.peek() != '{'){
				gps_measurements.clear();
				return gps_measurements;
			}
			stream.ignore(2, '{'); // Ignore '{'
			stream.ignore(2, '\n'); // Ignore '\n'

			while(stream.peek() != '}' && !reached_end){
				measurement temp_m;
				if(stream.eof()){ reached_end = true; gps_measurements.clear(); return gps_measurements; }
				stream >> temp_m.type;
				KATCH_STATE("Type: " << temp_m.type << "\n");
				stream.ignore(1, ' ');

				stream >> temp_m.traversal_time;
				KATCH_STATE("Traversal time: " << temp_m.traversal_time << "\n");
				stream.ignore(1, ' ');

				stream.read(buffer, 8);

				util::strptime(buffer, "%H:%M:%S", &temp_m.tm);
				KATCH_STATE("Buffer: " << temp_m.tm.tm_hour << ":" << temp_m.tm.tm_min << ":" << temp_m.tm.tm_sec << "\n");

				gps_measurements.push_back(std::move(temp_m));

				stream.ignore(2, '\n'); // Ignore '\n'
			}
			stream.ignore(2, '}'); // Ignore '}'
			stream.ignore(2, '\n'); // Ignore '\n'

			if(stream.eof()){reached_end = true;}
			return gps_measurements;
		}

		std::tm calc_tm(uint32_t total_sec)
		{
			std::tm result;
			uint32_t tm_sec = total_sec % 60;
			uint32_t total_min = floor(total_sec / 60);
			uint32_t tm_min = total_min % 60;
			uint32_t tm_hour = floor(total_min / 60);
			result.tm_sec = tm_sec;
			result.tm_min = tm_min;
			result.tm_hour = tm_hour;
			return result;
		}

        // Returns whether or not the histograms can be merged
		using dominance = katch::dominance;  //  { left_dom, right_dom, no_dom }
        bool can_merge(Histogram * const left, Histogram * const right)
        {
            // Check for dominance. Only merge if there are no dominant histogram
            return dominance::no_dom == katch::stochastic_dominance(left, right);
        }


        Histogram create_histogram(std::vector<measurement> measurements, uint32_t index)
		{
			uint32_t total_sec = index*SECONDS_IN_DYN_HISTOGRAM;
			tm start_tm = calc_tm(total_sec);
			tm end_tm = calc_tm(total_sec + SECONDS_IN_DYN_HISTOGRAM);

			std::map<uint32_t,double> buckets;

			uint32_t number_of_measurements = measurements.size();
			for (auto i = measurements.begin(); i != measurements.end(); ++i)
			{
				if(buckets.find(i->traversal_time) == buckets.end())
				{
					buckets[i->traversal_time] = 1;
				}
				else
				{
					buckets[i->traversal_time] ++;
				}
			}
			for (auto i = buckets.begin(); i != buckets.end(); ++i)
			{
				i->second = i->second*100/number_of_measurements;
			}
			return Histogram(start_tm, end_tm, number_of_measurements, buckets);
		}

        Histogram create_histogram_alldata(std::vector<measurement> measurements)
        {
            tm start_tm = calc_tm(0);
            tm end_tm = calc_tm(SECONDS_IN_DAY);

            std::map<uint32_t,double> buckets;

            uint32_t number_of_measurements = measurements.size();
            for (auto i = measurements.begin(); i != measurements.end(); ++i)
            {
                if(buckets.find(i->traversal_time) == buckets.end())
                {
                    buckets[i->traversal_time] = 1;
                }
                else
                {
                    buckets[i->traversal_time] ++;
                }
            }
            for (auto i = buckets.begin(); i != buckets.end(); ++i)
            {
                i->second = i->second*100/number_of_measurements;
            }
            return Histogram(start_tm, end_tm, number_of_measurements, buckets);
        }

        std::vector<Histogram> merge_qualified_histograms(std::vector<Histogram> const histograms)
        {
            // create a copy of the input vector as we will be removing elements as they are merged
            std::vector<Histogram> merged = histograms;
            // Go trough the vector backwards
            for (int i = merged.size(); i >= 2; --i)
            {
                if (can_merge(&merged[i-2], &merged[i-1]))
                {
                    merged[i-2].merge(merged[i-1]);
                    merged.erase(merged.begin()+i-1);
                }
            }
            return merged;
        }

        std::array<std::vector<measurement>, KATCH_NUMBER_OF_DYN_HISTOGRAMS > divide_measurements(std::vector<measurement>& gps_measurements)
		{
			std::array<std::vector<measurement>, KATCH_NUMBER_OF_DYN_HISTOGRAMS > measurements_split;

			assert(SECONDS_IN_DAY % KATCH_NUMBER_OF_DYN_HISTOGRAMS == 0);
            uint32_t divisor = SECONDS_IN_DYN_HISTOGRAM;

			for (auto it : gps_measurements )
			{
				uint32_t secs = it.tm.tm_sec + it.tm.tm_min*60 + it.tm.tm_hour*3600;
				uint32_t index = floor(secs / divisor);  // using the constant directly results in 0 results
				measurements_split[index].push_back(it);
			}

			return measurements_split;
		}

        std::vector<Edge> read_edges_alldata(std::ifstream& input_hist_file) {
            std::vector<Edge> result;
            bool reached_end = false;
            uint32_t amount_of_edges = 0;

            std::vector<measurement> gps_measurements;
            std::array<std::vector<measurement>, KATCH_NUMBER_OF_DYN_HISTOGRAMS > measurements_split;
            while(!input_hist_file.eof() && input_hist_file.good() && !reached_end)
            {
                input_hist_file.ignore(256, '#');
                Edge edge;

                uint32_t edge_id;
                uint32_t start_node;
                uint32_t target_node;
                double avgcost;

                input_hist_file >> edge_id;
                input_hist_file >> start_node;
                input_hist_file >> target_node;
                input_hist_file >> avgcost;
                input_hist_file.ignore(2, '\n'); // Ignore '\n'
                KATCH_STATE("edge_id: " << edge_id << "\n");
                KATCH_STATE("start_node: " << start_node << "\n");
                KATCH_STATE("target_node: " << target_node << "\n");
                KATCH_STATE("avg_cost: " << avgcost << "\n");

                edge.set_edge_id(edge_id);
                edge.set_source(start_node);
                edge.set_target(target_node);

                gps_measurements.clear();
                if (input_hist_file.peek() == '{' && !reached_end){
                    KATCH_STATE("Peeked and found no # \n");
                    gps_measurements = ReadMeasurements(input_hist_file, reached_end);
                    if(input_hist_file.eof()){reached_end = true; continue;}
                }
                KATCH_STATE("Done reading measurements for this node.\n");

                if(gps_measurements.size() == 0) {
                    measurement temp;
                    temp.traversal_time = avgcost;
                    gps_measurements.push_back(temp);
                }
                Histogram hist = create_histogram_alldata(gps_measurements);
                edge.set_cost( HistCost(hist));
                result.push_back(std::move(edge));
                amount_of_edges ++;
                KATCH_STATE("NUMBER OF EDGES: " << amount_of_edges << "\n");
            }
            KATCH_STATUS("Amount of edges: " << amount_of_edges << " \n");
            return result;
        }

		std::vector<Edge> read_edges(const std::string& input_file_name, TimeType time_type)
		{
            std::ifstream input_hist_file(input_file_name);
            std::vector<Edge> result;

			if (input_file_name == "")
			{
				KATCH_ERROR("Empty input file name given.\n");
				return result;
			}

			if (!input_hist_file.is_open())
			{
				KATCH_CONTINUE_STATUS(" ABORT\n");
				KATCH_ERROR("Unable to open file '" << input_file_name << "'\n");
				return result;
			}
            if (time_type == alldata) {
                return read_edges_alldata(input_hist_file);
            }
            else if (time_type == peak) {
                //
            }
            else if (time_type == days) {
                //
            }
            else {
                std::cout << "Invalid time type selected" << std::endl;
            }
		}
	}
}

#endif /* KATCH_HIST_FORMAT_H_ */
