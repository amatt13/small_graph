#ifndef KATCH_HIST_FORMAT_H_
#define KATCH_HIST_FORMAT_H_

#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <time.h>
#include <stdlib.h>

#include "histogram.h"
#include "edge.h"
#include "misc.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "hist_cost.h"

#define NUMBER_OF_DYN_HISTOGRAMS 96

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_DYN_HISTOGRAM SECONDS_IN_DAY/NUMBER_OF_DYN_HISTOGRAMS

namespace katch
{
    struct measurement{
        uint32_t type;
        uint32_t traversal_time;
        std::tm tm;
        uint32_t day_of_week;
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
				std::cerr << "TWO # AFTER EACHOTHER." << line << "\n";
				reached_end = true;
				gps_measurements.clear();
				return gps_measurements;
			}

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
				stream.ignore(1, ' ');

				stream >> temp_m.traversal_time;
                stream.ignore(1, ' ');

				stream.read(buffer, 8);

				util::strptime(buffer, "%H:%M:%S", &temp_m.tm);
                stream.ignore(1, ' ');

                stream >> temp_m.day_of_week;

				gps_measurements.push_back(std::move(temp_m));

				stream.ignore(2, '\n'); // Ignore '\n'
			}
			stream.ignore(2, '}'); // Ignore '}'
			stream.ignore(2, '\n'); // Ignore '\n'

			if(stream.eof()){reached_end = true;}
			return gps_measurements;
		}

        std::pair<std::vector<measurement>, std::vector<measurement>> ReadMeasurements_days(std::ifstream& stream, bool &reached_end)
        {
            std::vector<measurement> gps_measurements_weekdays;
            std::vector<measurement> gps_measurements_weekend;

            if(stream.peek() == '#'){
                std::string line;
                std::getline(stream, line);
                std::cerr << "TWO # AFTER EACHOTHER." << line << "\n";
                reached_end = true;
                gps_measurements_weekdays.clear();
                gps_measurements_weekend.clear();
                return std::make_pair(gps_measurements_weekdays, gps_measurements_weekend);
            }

            char buffer[10];

            if(stream.peek() != '{'){
                gps_measurements_weekdays.clear();
                gps_measurements_weekend.clear();
                return std::make_pair(gps_measurements_weekdays, gps_measurements_weekend);
            }
            stream.ignore(2, '{'); // Ignore '{'
            stream.ignore(2, '\n'); // Ignore '\n'

            while(stream.peek() != '}' && !reached_end){
                measurement temp_m;
                if(stream.eof()){
                    reached_end = true;
                    gps_measurements_weekdays.clear();
                    gps_measurements_weekend.clear();
                    return std::make_pair(gps_measurements_weekdays, gps_measurements_weekend);
                }
                stream >> temp_m.type;
                stream.ignore(1, ' ');

                stream >> temp_m.traversal_time;
                stream.ignore(1, ' ');

                stream.read(buffer, 8);
                util::strptime(buffer, "%H:%M:%S", &temp_m.tm);
                stream.ignore(1, ' ');

                stream >> temp_m.day_of_week;

                // 0 is monday, 6 is sunday
                if (temp_m.day_of_week < 5) {
                    gps_measurements_weekdays.push_back(std::move(temp_m));
                } else if (temp_m.day_of_week == 5 || temp_m.day_of_week == 6) {
                    gps_measurements_weekend.push_back(std::move(temp_m));
                }
                else {
                    assert(false);
                }

                stream.ignore(2, '\n'); // Ignore '\n'
            }
            stream.ignore(2, '}'); // Ignore '}'
            stream.ignore(2, '\n'); // Ignore '\n'

            if(stream.eof()) {
                reached_end = true;
            }
            return std::make_pair(gps_measurements_weekdays, gps_measurements_weekend);
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

        Histogram create_histogram_peak(std::vector<measurement> measurements, tm start_tm, tm end_tm)
		{
			std::map<uint32_t,double> buckets;

			auto number_of_measurements = measurements.size();
			for (auto i = measurements.begin(); i != measurements.end(); ++i) {
				if(buckets.find(i->traversal_time) == buckets.end()) {
					buckets[i->traversal_time] = 1;
				}
				else {
					buckets[i->traversal_time] ++;
				}
			}
			for (auto i = buckets.begin(); i != buckets.end(); ++i) {
				i->second = i->second*100/number_of_measurements;
			}
			return Histogram(start_tm, end_tm, number_of_measurements, buckets);
		}

        Histogram create_histogram_alldata(std::vector<measurement> measurements)
        {
            tm start_tm = calc_tm(0);
            tm end_tm = calc_tm(SECONDS_IN_DAY);

            std::map<uint32_t,double> buckets;

            auto number_of_measurements = measurements.size();
            for (auto i = measurements.begin(); i != measurements.end(); ++i) {
                if(buckets.find(i->traversal_time) == buckets.end()) {
                    buckets[i->traversal_time] = 1;
                }
                else {
                    buckets[i->traversal_time] ++;
                }
            }
            for (auto i = buckets.begin(); i != buckets.end(); ++i) {
                i->second = i->second*100/number_of_measurements;
            }
            return Histogram(start_tm, end_tm, number_of_measurements, buckets);
        }

        Histogram create_histogram_days(std::vector<measurement> measurements, uint32_t index)
        {
            auto total_sec = index*SECONDS_IN_DYN_HISTOGRAM;
            tm start_tm = calc_tm(total_sec);
            tm end_tm = calc_tm(total_sec + SECONDS_IN_DYN_HISTOGRAM);

            std::map<uint32_t,double> buckets;

            auto number_of_measurements = measurements.size();
            for (auto i = measurements.begin(); i != measurements.end(); ++i) {
                if(buckets.find(i->traversal_time) == buckets.end()) {
                    buckets[i->traversal_time] = 1;
                }
                else {
                    buckets[i->traversal_time] ++;
                }
            }
            for (auto i = buckets.begin(); i != buckets.end(); ++i) {
                i->second = i->second*100/number_of_measurements;
            }
            return Histogram(start_tm, end_tm, number_of_measurements, buckets);
        }

        std::array<std::vector<measurement>, NUMBER_OF_DYN_HISTOGRAMS > divide_measurements(std::vector<measurement>& gps_measurements)
		{
			std::array<std::vector<measurement>, NUMBER_OF_DYN_HISTOGRAMS > measurements_split;

			assert(SECONDS_IN_DAY % NUMBER_OF_DYN_HISTOGRAMS == 0);
            auto divisor = SECONDS_IN_DYN_HISTOGRAM;

			for (auto it : gps_measurements )
			{
				auto secs = it.tm.tm_sec + it.tm.tm_min*60 + it.tm.tm_hour*3600;
				auto index = floor(secs / divisor);  // using the constant directly results in 0 results
				measurements_split[index].push_back(it);
			}

			return measurements_split;
		}

        /* multiple histograms. 5 for weekdays, 1 for weekends
         * weekdays: mon-fri
         * 1 00:00 - 07:00 off
         * 2 07:00 - 08:30 peak
         * 3 08:30 - 15:00 off
         * 4 15:00 - 17:00 peak
         * 5 17:00 - 24:00 off
         * weekends: sun-sat
         * 6 00:00 - 24:00
        */
        /**
         * peak version
         * The measurement struct has a weekday field:  Monday is 0 and Sunday is 6. We do NOT use the tm to find the weekday
         * */
        std::array<std::vector<measurement>, 6 > divide_measurements_peak(std::vector<measurement>& gps_measurements)
        {
            auto time1 = peak1.tm_sec + peak1.tm_min*60 + peak1.tm_hour*3600;
            auto time2 = peak2.tm_sec + peak2.tm_min*60 + peak2.tm_hour*3600;
            auto time3 = peak3.tm_sec + peak3.tm_min*60 + peak3.tm_hour*3600;
            auto time4 = peak4.tm_sec + peak4.tm_min*60 + peak4.tm_hour*3600;
            auto time5 = peak5.tm_sec + peak5.tm_min*60 + peak5.tm_hour*3600;

            std::array<std::vector<measurement>, 6 > measurements_split;

            for (auto it : gps_measurements )
            {
                auto secs = it.tm.tm_sec + it.tm.tm_min*60 + it.tm.tm_hour*3600;

                if (it.day_of_week == 5 || it.day_of_week == 6) {
                    measurements_split[5].push_back(it); //weekend
                } else if (secs >= time1 && secs <=time2) {
                    measurements_split[0].push_back(it); // 00:00 - 07:00 off
                } else if (secs >= time2 && secs <=time3) {
                    measurements_split[1].push_back(it); // 07:00 - 08:30 peak
                } else if (secs >= time3 && secs <=time4) {
                    measurements_split[2].push_back(it); // 08:30 - 15:00 off
                } else if (secs >= time4 && secs <=time5) {
                    measurements_split[3].push_back(it); // 15:00 - 17:00 peak
                } else if (secs >= time5) {
                    measurements_split[4].push_back(it); // 17:00 - 24:00 off
                }
                else {
                    assert(false);
                }
            }

            return measurements_split;
        }

        std::pair<std::vector<Edge>, std::string> read_edges_days(std::ifstream& input_hist_file) {
            std::vector<Edge> result;
            bool reached_end = false;
            uint32_t amount_of_edges = 0;

            std::vector<measurement> gps_measurements_weekdays;
            std::vector<measurement> gps_measurements_weekends;
            std::vector<Histogram> histograms;
            std::array<std::vector<measurement>, NUMBER_OF_DYN_HISTOGRAMS > measurements_split_weekdays;
            std::array<std::vector<measurement>, NUMBER_OF_DYN_HISTOGRAMS > measurements_split_weekends;

            int type = -1;
            while(!input_hist_file.eof() && input_hist_file.good() && !reached_end)
            {
                input_hist_file.ignore(256, '#');
                Edge edge{};

                uint32_t edge_id;
                uint32_t start_node;
                uint32_t target_node;
                double avgcost;

                input_hist_file >> edge_id;
                input_hist_file >> start_node;
                input_hist_file >> target_node;
                input_hist_file >> avgcost;
                input_hist_file.ignore(2, '\n'); // Ignore '\n'

                edge.set_edge_id(edge_id);
                edge.set_source(start_node);
                edge.set_target(target_node);

                gps_measurements_weekdays.clear();
                gps_measurements_weekends.clear();
                if (input_hist_file.peek() == '{' && !reached_end){
                    auto mess = ReadMeasurements_days(input_hist_file, reached_end);
                    gps_measurements_weekdays = mess.first;
                    gps_measurements_weekends = mess.second;
                    if(input_hist_file.eof()){reached_end = true; continue;}
                }
                if (gps_measurements_weekdays.size() != 0) {
                    type = gps_measurements_weekdays[0].type;
                } else if (gps_measurements_weekends.size() != 0) {
                    type = gps_measurements_weekends[0].type;
                }

                measurements_split_weekdays = divide_measurements(gps_measurements_weekdays);
                measurements_split_weekends = divide_measurements(gps_measurements_weekends);
                histograms.clear();
                uint32_t index = 0;

                // weekdays first
                for(auto it : measurements_split_weekdays ){
                    if(it.size() == 0){
                        measurement temp;
                        temp.type = -1;
                        temp.traversal_time = avgcost;
                        it.push_back(temp);
                    }
                    Histogram hist = create_histogram_days(it, index);
                    histograms.push_back(std::move(hist));
                    index ++;
                }
                // weekends last
                index = 0;
                for(auto it : measurements_split_weekends ){
                    if(it.size() == 0){
                        measurement temp;
                        temp.traversal_time = avgcost;
                        it.push_back(temp);
                    }
                    Histogram hist = create_histogram_days(it, index);
                    histograms.push_back(std::move(hist));
                    index ++;
                }
                edge.set_cost( HistCost(histograms));
                result.push_back(std::move(edge));
                amount_of_edges ++;
            }
            std::cout << "Amount of edges: " << amount_of_edges << " \n";
            return std::make_pair(std::move(result), util::get_location_fromgraphtype(type));
        }

        // read edges and construct peak histograms
        std::pair<std::vector<Edge>, std::string> read_edges_peak(std::ifstream& input_hist_file) {
            std::vector<Edge> result;
            bool reached_end = false;
            auto amount_of_edges = 0;

            std::vector<measurement> gps_measurements;
            std::vector<Histogram> histograms;
            std::array<std::vector<measurement>, 6 > measurements_split;

            auto type = -1;
            while(!input_hist_file.eof() && input_hist_file.good() && !reached_end)
            {
                input_hist_file.ignore(256, '#');
                Edge edge{};

                uint32_t edge_id;
                uint32_t start_node;
                uint32_t target_node;
                double avgcost;

                input_hist_file >> edge_id;
                input_hist_file >> start_node;
                input_hist_file >> target_node;
                input_hist_file >> avgcost;
                input_hist_file.ignore(2, '\n'); // Ignore '\n'

                edge.set_edge_id(edge_id);
                edge.set_source(start_node);
                edge.set_target(target_node);

                gps_measurements.clear();
                if (input_hist_file.peek() == '{' && !reached_end){
                    gps_measurements = ReadMeasurements(input_hist_file, reached_end);
                    if(input_hist_file.eof()){reached_end = true; continue;}
                }

                if (gps_measurements.size() != 0) {
                    type = gps_measurements[0].type;
                }
                measurements_split = divide_measurements_peak(gps_measurements);
                histograms.clear();

                for(auto i = 0; i <= 5; i++){
                    if( measurements_split[i].size() == 0){
                        measurement temp;
                        temp.type = -1;
                        temp.traversal_time = avgcost;
                        measurements_split[i].push_back(temp);
                    }
                    if (i == 0) {
                        Histogram hist = create_histogram_peak(measurements_split[i], peak1, peak2);
                        histograms.push_back(std::move(hist));
                    } else if (i == 1) {
                        Histogram hist = create_histogram_peak(measurements_split[i], peak2, peak3);
                        histograms.push_back(std::move(hist));
                    } else if (i == 2) {
                        Histogram hist = create_histogram_peak(measurements_split[i], peak3, peak4);
                        histograms.push_back(std::move(hist));
                    } else if (i == 3) {
                        Histogram hist = create_histogram_peak(measurements_split[i], peak4, peak5);
                        histograms.push_back(std::move(hist));
                    } else if (i == 4) {
                        Histogram hist = create_histogram_peak(measurements_split[i], peak5, day_end);
                        histograms.push_back(std::move(hist));
                    } else if (i == 5) {
                        Histogram hist = create_histogram_peak(measurements_split[i], day_start, day_end);
                        histograms.push_back(std::move(hist));
                    } else {
                        assert(false);
                    }
                }
                edge.set_cost( HistCost(histograms));
                result.push_back(std::move(edge));
                amount_of_edges ++;
            }
            std::cout << "Amount of edges: " << amount_of_edges << " \n";
            return std::make_pair(std::move(result), util::get_location_fromgraphtype(type));
        }

        // read edges and construct alldata histograms
        std::pair<std::vector<Edge>, std::string> read_edges_alldata(std::ifstream& input_hist_file) {
            std::vector<Edge> result;
            bool reached_end = false;
            uint32_t amount_of_edges = 0;

            std::vector<measurement> gps_measurements;
            std::array<std::vector<measurement>, NUMBER_OF_DYN_HISTOGRAMS > measurements_split;

            auto type = -1;
            while(!input_hist_file.eof() && input_hist_file.good() && !reached_end)
            {
                input_hist_file.ignore(256, '#');
                Edge edge{};

                uint32_t edge_id;
                uint32_t start_node;
                uint32_t target_node;
                double avgcost;

                input_hist_file >> edge_id;
                input_hist_file >> start_node;
                input_hist_file >> target_node;
                input_hist_file >> avgcost;
                input_hist_file.ignore(2, '\n'); // Ignore '\n'n");

                edge.set_edge_id(edge_id);
                edge.set_source(start_node);
                edge.set_target(target_node);

                gps_measurements.clear();
                if (input_hist_file.peek() == '{' && !reached_end){
                    gps_measurements = ReadMeasurements(input_hist_file, reached_end);
                    if(input_hist_file.eof()){reached_end = true; continue;}
                }

                if(gps_measurements.size() == 0) {
                    measurement temp;
                    temp.traversal_time = avgcost;
                    temp.type = -1;
                    gps_measurements.push_back(temp);
                }
                else {
                    type = gps_measurements[0].type;
                }
                auto hist = create_histogram_alldata(gps_measurements);
                edge.set_cost( HistCost(hist));
                result.push_back(std::move(edge));
                amount_of_edges ++;
            }
            std::cout << "Amount of edges: " << amount_of_edges << " \n";
            return std::make_pair(std::move(result), util::get_location_fromgraphtype(type));
        }

		std::pair<std::vector<Edge>, std::string> read_edges(const std::string& input_file_name, TimeType time_type)
		{
            std::ifstream input_hist_file(input_file_name);
            std::vector<Edge> result;

			if (input_file_name.empty()) {
                std::cerr << "Empty input file name given.\n";
				return std::make_pair(std::move(result), util::get_location_fromgraphtype(-1));
			}
			if (!input_hist_file.is_open()) {
                std::cerr << " ABORT\n";
                std::cout << "Unable to open file '" << input_file_name << "'\n";
				return std::make_pair(std::move(result), util::get_location_fromgraphtype(-1));
			}

            // file is OK. Start reading it
            if (time_type == alldata) {
                std::cout << "reading alldata"<< std::endl;
                return read_edges_alldata(input_hist_file);
            }
            else if (time_type == peak) {
                std::cout << "reading peak"<< std::endl;
                return read_edges_peak(input_hist_file);
            }
            else if (time_type == days) {
                std::cout << "reading days"<< std::endl;
                return read_edges_days(input_hist_file);
            }
            else {
                std::cout << "Invalid time type selected" << std::endl;
                return std::make_pair(std::move(result), util::get_location_fromgraphtype(-1));
            }
		}
	}
}

#endif /* KATCH_HIST_FORMAT_H_ */
