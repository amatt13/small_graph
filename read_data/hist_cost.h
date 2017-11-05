//
// Created by simon on 3/28/17.
//

#ifndef KATCH_HIST_COST_H
#define KATCH_HIST_COST_H


#include <vector>
#include "histogram.h"

namespace katch {

    using Histogram = katch::Histogram;
    using time = struct tm;

    class HistCost {

    private:
        std::vector<Histogram> _histograms;
        // Lower and upper bound used for histogram merging.
        uint32_t lower_bound;
        uint32_t upper_bound;

    public:
        // empty
        HistCost() = default;
        ~HistCost() = default;

        // full
        HistCost(uint32_t upper, uint32_t lower, std::vector<Histogram> hist) :
                upper_bound(upper), lower_bound(lower), _histograms(hist) {}

        // vetor of histograms
        HistCost(std::vector<Histogram> hist):_histograms(hist){
            uint32_t min = INT32_MAX, max = 0;
            for (auto hist : _histograms) {
                if (hist.get_max_cost() > max) {
                    max = hist.get_max_cost();
                }
                if (hist.get_min_cost() > min) {
                    max = hist.get_min_cost();
                }
            }
        };

        // single histogram
        HistCost(Histogram hist){
            std::vector<Histogram> hists = {hist};
            _histograms = hists;
            uint32_t min = INT32_MAX, max = 0;
            if (hist.get_max_cost() > max) {
                max = hist.get_max_cost();
            }
            if (hist.get_min_cost() > min) {
                max = hist.get_min_cost();
            }
        };

        uint32_t get_complexity(){
            return _histograms.size();
        }

        void set_histograms(std::vector<Histogram> histograms) { _histograms = histograms; }

        std::vector<Histogram> get_histograms() const { return _histograms; }

        void add_cost(Histogram cost)
        {
            _histograms.push_back(cost);
        }

        // Constant cost is interpreted as having a single histogram (and thus not being time-dependent).
        bool is_constant()const {return _histograms.size()==1;}

        Histogram& get_constant_value() {
            assert (is_constant());
            return _histograms[0];
        }

        Histogram const get_constant_value() const {
            assert (is_constant());
            return _histograms[0];
        }

        void set_constant_value(Histogram value){
            _histograms.clear();
            _histograms.push_back(value);
        }

        uint32_t size() const{
            return _histograms.size();
        }

        // get the histograms that covers the given time span
        std::vector<Histogram> get_histograms_from_timespan(std::pair<uint16_t, uint16_t> time_span) {
            auto timespan_start = time_span.first;
            auto timespan_end = time_span.second;
            auto histograms = get_histograms();
            std::vector<Histogram> candidate_histograms;

            for (auto h = histograms.begin(); h != histograms.end(); ++h) {
                auto start = katch::util::get_total_seconds(h->get_tp_start());
                auto end = katch::util::get_total_seconds(h->get_tp_end());
                if (timespan_start >= start && timespan_end <= end) {
                    candidate_histograms.push_back(*h);
                }
            }
            return candidate_histograms;
        }

        // get the histogram that covers the given point in time
        std::pair<Histogram, uint32_t> get_histogram_from_time(struct tm point_in_time) {
            auto timestamp = katch::util::get_total_seconds(point_in_time);
            auto histograms = get_histograms();

            for (uint32_t h = 0; h != histograms.size(); ++h) {
                auto start = katch::util::get_total_seconds(histograms[h].get_tp_start());
                auto end = katch::util::get_total_seconds(histograms[h].get_tp_end());
                assert(timestamp >= start && timestamp <= end);
                return std::make_pair(histograms[h], h);

                    // you should always find an histogram

            }
        }

        Histogram *eval(time time)  // Biver ikke kaldt pt
        {
            for (auto it = _histograms.begin(); it != _histograms.end(); ++it) {
                if (time.tm_hour >= it->get_tp_start().tm_hour &&
                    time.tm_min >= it->get_tp_start().tm_min &&
                    time.tm_sec >= it->get_tp_start().tm_sec &&
                    time.tm_hour < it->get_tp_end().tm_hour &&
                    time.tm_hour < it->get_tp_end().tm_hour &&
                    time.tm_hour < it->get_tp_end().tm_hour)
                    return &(*it);
            }
            return new Histogram();
        }

        uint32_t get_lower() const{
            return lower_bound;
        }

        uint32_t get_upper() const{
            return upper_bound;
        }
    };

    // combines two histograms for the same time periods.
    // links lhs to rhs to make new histogram: w --(lhs)-->v--(rhs)-->l turns to w--(result)-->l
    Histogram* linkhist(const Histogram * lhs, const Histogram * rhs) {
        Histogram *result = new Histogram();
        auto lhs_buckets = lhs->get_buckets();
        auto rhs_buckets = rhs->get_buckets();
        double temp_prob;
        for (auto lit = lhs_buckets.begin(); lit != lhs_buckets.end(); ++lit) {
            for (auto rit = rhs_buckets.begin(); rit != rhs_buckets.end(); ++rit) {
                temp_prob = result->get_probability(lit->first + rit->first);
                if (temp_prob != -1) {
                    result->set_probability(lit->first + rit->first, temp_prob + (lit->second * rit->second));
                } else {
                    result->set_probability(lit->first + rit->first, lit->second * rit->second);
                }
            }
        }
        result->set_n_measurements(lhs->get_n_measurements() + rhs->get_n_measurements());

        assert(lhs->get_tp_start().tm_hour == rhs->get_tp_start().tm_hour);
        assert(lhs->get_tp_start().tm_min == rhs->get_tp_start().tm_min);
        assert(lhs->get_tp_start().tm_sec == rhs->get_tp_start().tm_sec);

        assert(lhs->get_tp_end().tm_hour == rhs->get_tp_end().tm_hour);
        assert(lhs->get_tp_end().tm_min == rhs->get_tp_end().tm_min);
        assert(lhs->get_tp_end().tm_sec == rhs->get_tp_end().tm_sec);

        result->set_tp_start(lhs->get_tp_start());
        result->set_tp_end(lhs->get_tp_end());
        assert(result->validate_probability());
        return result;
    }

    Histogram convolute(Histogram *H1, Histogram *H2) {
        std::map<uint32_t, double> H1_buckets = H1->get_buckets();
        std::map<uint32_t, double> H2_buckets = H2->get_buckets();

        std::map<uint32_t, double> combination_buckets;

        for (std::pair<uint32_t, double> H1_bucket: H1_buckets) {
            for (std::pair<uint32_t, double> H2_bucket: H2_buckets) {
                // Collect all cost combinations in a vector
                uint32_t cost = H1_bucket.first + H2_bucket.first;
                if (combination_buckets.find(cost) != combination_buckets.end())  // Is this bucket already present?
                {
                    // add to bucket
                    combination_buckets[cost] += (H1_bucket.second * H2_bucket.second) / 100;
                } else {
                    // new bucket
                    combination_buckets[cost] = (H1_bucket.second * H2_bucket.second) / 100;
                }
            }
        }

        // decide time interval for the new histogram
        struct tm start, end;
        if (util::time_before(H1->get_tp_start(), H2->get_tp_start())) {
            start = H1->get_tp_start();
        } else {
            start = H2->get_tp_start();
        }
        if (util::time_after(H1->get_tp_end(), H2->get_tp_end())) {
            end = H1->get_tp_end();
        } else {
            end = H2->get_tp_end();
        }

        Histogram *result = new Histogram(start, end, (H1->get_n_measurements() + H2->get_n_measurements()),
                                          combination_buckets);
        assert(result->validate_probability());
        return *result;
    }

    template<typename EdgeCost>
    EdgeCost* convolute_self(EdgeCost *edge) {
        std::vector<Histogram> histograms = edge->get_histograms();

        if (histograms.size() == 1) {
            return edge; // base case : trivial
        } else {
            Histogram base = histograms[0];
            // for every histogram after the first
            for (int i = 1; i != histograms.size(); ++i) {
                base = convolute(&base, &histograms[i]);
            }
            std::vector<Histogram> result = {base};
            edge->set_histograms(result);
            return edge;
        }
    }

    // return the span which the histogram covers in time given an start time
    std::pair<uint16_t, uint16_t> get_time_span(Histogram histogram, struct tm start_time) {
        auto buckets = histogram.get_buckets();
        uint16_t earliest = katch::util::get_total_seconds(start_time);
        uint16_t latest = katch::util::get_total_seconds(start_time);
        uint32_t highest = 0, lowest = UINT32_MAX;

        //lowest is set to an high number such that latest will always be of an lower value

        if (buckets.size() == 0)
            return std::make_pair(earliest,
                                  latest);  //TODO: if the buckets are empty, can we then be sure that the time is initialized?

        for (auto i = buckets.begin(); i !=
                                       buckets.end(); ++i)  // Find the buckets with the highest and lowest cost //TODO: if they are ordered, can we then take the first and last bucket?
        {

            if (i->first + katch::util::get_total_seconds(start_time) > highest) {
                highest = i->first;
            }
            if (i->first < lowest) {
                lowest = i->first;
            }
        }
        // earliest + lowest --> start, latest + highest --> end
        return std::make_pair(earliest + lowest, latest + highest);
    }


    double confidence(std::pair<uint16_t, uint16_t> time_span, struct tm start_time, Histogram *base,
                      Histogram *edge_histogram) {
        auto edge_start_time = katch::util::get_total_seconds(edge_histogram->get_tp_start());
        auto edge_end_time = katch::util::get_total_seconds(edge_histogram->get_tp_end());
        // trivial case : outside of span
        if (edge_start_time > time_span.second || edge_end_time < time_span.first) {
            return 0.0;
        }
            // trivial case : is covering time span
        else if (edge_start_time <= time_span.first && edge_end_time >= time_span.second) {
            return 1.0;
        }
            // complex case : covers partly
        else {
            auto start_time_seconds = katch::util::get_total_seconds(start_time);
            double confidence = 0.0;
            auto buckets = base->get_buckets();

            for (auto it = buckets.begin(); it != buckets.end(); ++it) {
                // is the edge histogram reached by this start time and travel cost?
                auto time_point = it->first + start_time_seconds;
                if (time_point >= edge_start_time && time_point <= edge_end_time) {
                    confidence += it->second /
                                  100;  // divided by hundred in order to change 50.0 to 0.5 (confidence is [0-1])
                }
            }
            return confidence;
        }
    }

    /*
    Det her vil ikke tage imod _edges fra path klassen. Den klasse bruger uints til at repræssentere sine edges med.
    Vi bør ændre path klassen (tch_ea_qury.h)
    */
    // Takes an edge that represents an already convoluted path.
    template<typename EdgeCost>
    EdgeCost* convolute_path(EdgeCost* path, EdgeCost* to_add_edge, struct tm start_time) {
        auto base = path->get_histogram_from_time(
                start_time);  // get the histogram that covers the start time together with its position in path
        Histogram base_histogram = base.first;
        uint32_t base_histogram_position = base.second;

        auto time_span = get_time_span(base_histogram,
                                       start_time);  // get the earliest and latest points in time. Used to find relevant histograms
        std::vector<Histogram> to_be_convoluted = to_add_edge->get_histograms_from_timespan(
                time_span);  // get the histogram(s) that can be reached
        // convolve the histogram that has the exacts same time period first (if it exists)
        for (int i = 0; i != to_be_convoluted.size(); ++i) {
            if (katch::util::time_equal(base_histogram.get_tp_start(), to_be_convoluted[i].get_tp_start()) &&
                katch::util::time_equal(base_histogram.get_tp_end(), to_be_convoluted[i].get_tp_end())) {
                base_histogram = convolute(&base_histogram, &to_be_convoluted[i]);
                to_be_convoluted.erase( to_be_convoluted.begin() + i );
                break;
            }
        }

        // collect all histograms that don't have the same time periods
        std::vector< std::pair<katch::Histogram, double> > histogram_confidence_list = {};
        for (auto &j : to_be_convoluted) {
            auto c = confidence(time_span, start_time, &base_histogram, &j);
            if (c > 0.0) {
                histogram_confidence_list.push_back(std::make_pair(j, c));
            }
        }

        // merge all in list before merging list with self
        std::vector<katch::Histogram> merged_histograms;
        for (std::pair<katch::Histogram, double> hist_with_conf : histogram_confidence_list)
        {
            // merge all histograms in vector
            Histogram base_copy = base_histogram;
            // create an copy of the base histogram and apply confidence
            base_copy = base_copy * (1.0d -  (hist_with_conf.second));
            // this can result in base_copy being multiplied by 0
            hist_with_conf.first = hist_with_conf.first * hist_with_conf.second;
            auto result = hist_with_conf.first.merge(base_copy);
            merged_histograms.push_back(result);  // merge with confidence
        }

        for (auto l = merged_histograms.begin(); l != merged_histograms.end(); ++l) {
            base_histogram = convolute(&base_histogram, &*l);
        }

        auto histograms = path->get_histograms();
        histograms[base_histogram_position] = base_histogram;
        path->set_histograms(histograms);

        return path;
    }

    //TODO man kunne godt lav en funktion der håndtere loopet nedenunder. Der sker det samme i convolute_path
    HistCost convolute_hist_cost (HistCost * lhs, HistCost * rhs) {
        auto lhs_histograms = lhs->get_histograms(), rhs_histograms = rhs->get_histograms();
        std::vector<Histogram> convoluted;
        std::vector< std::pair<katch::Histogram, double> > histogram_confidence_list = {};

        for (auto lhs_histogram : lhs_histograms) {
            // get the earliest and latest points in time. Used to find relevant histograms
            auto time_span = get_time_span(lhs_histogram, util::MIN_TIME);
            // get the histogram(s) that can be reached
            std::vector<Histogram> to_be_convoluted = rhs->get_histograms_from_timespan(time_span);

            // convolve the histogram that has the exacts same time period first (if it exists)
            for (int i = 0; i != to_be_convoluted.size(); ++i) {
                if (katch::util::time_equal(lhs_histogram.get_tp_start(), to_be_convoluted[i].get_tp_start()) &&
                    katch::util::time_equal(lhs_histogram.get_tp_end(), to_be_convoluted[i].get_tp_end())) {
                    lhs_histogram = convolute(&lhs_histogram, &to_be_convoluted[i]);
                    to_be_convoluted.erase( to_be_convoluted.begin() + i );
                    break;
                }
            }

            // collect all histograms that don't have the same time periods
            for (auto &candidate : to_be_convoluted) {
                auto c = confidence(time_span, util::MIN_TIME, &lhs_histogram, &candidate);
                if (c > 0.0) {
                    histogram_confidence_list.push_back(std::make_pair(candidate, c));
                }
            }

            // merge all in list before merging list with self
            std::vector<katch::Histogram> merged_histograms;
            for (auto hist_with_conf : histogram_confidence_list) {
                // merge all histograms in vector
                Histogram lhs_copy = lhs_histogram;
                // create an copy of the base histogram and apply confidence
                lhs_copy = lhs_copy * (1.0d -  (hist_with_conf.second));
                // this can result in lhs_copy being multiplied by 0
                hist_with_conf.first = hist_with_conf.first * hist_with_conf.second;
                auto result = hist_with_conf.first.merge(lhs_copy);
                merged_histograms.push_back(result);  // merge with confidence
            }

            for (auto &merged : merged_histograms) {
                lhs_histogram = convolute(&lhs_histogram, &merged);
            }
            convoluted.push_back(lhs_histogram);
            histogram_confidence_list.clear();
        }
        return HistCost(convoluted);
    }
}

#endif //KATCH_HIST_COST_H
