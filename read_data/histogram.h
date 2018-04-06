#ifndef KATCH_HISTOGRAM
#define KATCH_HISTOGRAM

#include <iostream>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <cmath>
#include <limits>
#include <vector>
#include <map>

#include "misc.h"
#include "double.h"


namespace katch {
    /**
     * Class for storing the cost probabilities for an edge, represented as an histogram
     */
    class Histogram {
        using time = struct tm;

        private:
        time _tp_start; // Histogram start period
        time _tp_end;   // Histogram end period
        uint32_t _n_measurements;   // Number of measurement that are used to construct the histogram
        bool _is_constant = false;  // if _n_measurements = 0, there will only be one bucket with the cost equal to the speed limit times length
        std::map<uint32_t,double> _buckets;  // cost, probability

        public:
        // Not all of the histogram constructors are used in this version of the program
        Histogram() = default;

        // start, end, num of measurements, buckets
        Histogram(time start, time end, bool is_constant, uint32_t measurements, std::map<uint32_t, double> buckets)
                : _tp_start(start), _tp_end(end), _is_constant(is_constant), _n_measurements(measurements), _buckets(buckets) {
            assert(validate_probability());
        }

        // start, end, num of measurements, buckets
        Histogram(time start, time end, uint32_t measurements, std::map<uint32_t, double> buckets)
        : _tp_start(start), _tp_end(end), _n_measurements(measurements), _buckets(buckets) {
            assert(validate_probability());
        }

        // num of measurements, buckets
        Histogram(uint32_t measurements, std::map<uint32_t, double> buckets)
        : _n_measurements(measurements), _buckets(buckets) { }

        // start, end
        Histogram(time start, time end)
        : _tp_start(start), _tp_end(end) { }

        // start, end, buckets
        Histogram(time start, time end, std::map<uint32_t, double> buckets)
        : _tp_start(start), _tp_end(end), _buckets(buckets) { }

        // start, end, constant_value, buckets
        Histogram(time start, time end, double constant, std::map<uint32_t, double> buckets, bool is_constant)
        : _tp_start(start), _tp_end(end), _buckets(buckets), _is_constant(is_constant) {
            _buckets[constant] = 100;
        }

        time get_tp_start() const {
            return _tp_start;
        }

        std::map<uint32_t, double> get_buckets() const {
            return _buckets;
        }

        /**
         * Validate the buckets. The sumn of all probabilities should equal 100%.
         * An error margin of +- katch::EPSILON is acceptable
         * @return True if valid, False if not
         */
        bool validate_probability() {
            double probability = 0.0;
            double goal = 100.0;
            std::vector<int> keys;
            for(auto it : _buckets ) {
                keys.push_back(it.first);
            }

            for (auto i = 0; i != keys.size(); i++) {
                probability += _buckets[keys[i]];
            }

            return (goal == probability ||
            std::abs(goal - probability) < std::abs(std::min(goal, probability))*katch::EPSILON);
        }

        uint32_t get_min_cost() {
            // map containers keep their elements ordered at all times, begin() points to the element that goes first
            return _buckets.begin()->first;
        }

        uint32_t get_max_cost() {
            uint32_t max = 0;
            for (auto it =_buckets.begin(); it!=_buckets.end(); ++it) {
                assert(it->first>=max);
                max = it->first;
            }
            return max;
        }
    };
}

#endif /* 'KATCH_HISTOGRAM */
