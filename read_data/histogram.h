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
    class Histogram {
        using time = struct tm;

        private:
        time _tp_start;
        time _tp_end;
        uint32_t _n_measurements;
        bool _is_constant = false;  // if _n_measurements = 0, there will only be one bucket with the cost equal to the speed limit times length
        std::map<uint32_t,double> _buckets;  // cost, probability
        public:
        static const Histogram INFTY;

        Histogram() = default;

        // start, end, num of measurements, buckets
        Histogram(time start, time end, bool is_constant, uint32_t measurements, std::map<uint32_t, double> buckets)
                : _tp_start(start), _tp_end(end), _is_constant(is_constant), _n_measurements(measurements), _buckets(buckets)
        {
            assert(validate_probability());
        }

        // start, end, num of measurements, buckets
        Histogram(time start, time end, uint32_t measurements, std::map<uint32_t, double> buckets)
        : _tp_start(start), _tp_end(end), _n_measurements(measurements), _buckets(buckets)
        {
            assert(validate_probability());
        }

        // num of measurements, buckets
        Histogram(uint32_t measurements, std::map<uint32_t, double> buckets)
        : _n_measurements(measurements), _buckets(buckets)
        { }

        // start, end
        Histogram(time start, time end)
        : _tp_start(start), _tp_end(end)
        {}

        // start, end, buckets
        Histogram(time start, time end, std::map<uint32_t, double> buckets)
        : _tp_start(start), _tp_end(end), _buckets(buckets)
        {}

        // start, end, constant_value, buckets
        Histogram(time start, time end, double constant, std::map<uint32_t, double> buckets, bool is_constant)
        : _tp_start(start), _tp_end(end), _buckets(buckets), _is_constant(is_constant)
        {_buckets[constant] = 100;}

        void set_probability(uint32_t value, double probability)
        {
            _buckets[value] = probability;
        }

        double get_probability(uint32_t bucket) const
        {
            std::map<uint32_t, double>::const_iterator it = _buckets.find(bucket);
            if (it != _buckets.end())
            {
                return it->second;
            }
            else
            {
                return -1;
            }
        }

        void set_n_measurements(uint32_t measurements)
        {
            _n_measurements = measurements;
        }

        uint32_t get_n_measurements() const
        {
            return _n_measurements;
        }

        void set_tp_start(time tp_start)
        {
            _tp_start = tp_start;
        }

        time get_tp_start() const
        {
            return _tp_start;
        }

        void set_tp_end(time tp_end)
        {
            _tp_end = tp_end;
        }

        time get_tp_end() const
        {
            return _tp_end;
        }

        uint64_t get_bucket_size() const
        {
            return _buckets.size();
        }

        void set_buckets(std::map<uint32_t, double> buckets)
        {
            _buckets = buckets;
        }

        std::map<uint32_t, double> get_buckets() const
        {
            return _buckets;
        }

        bool is_constant() const { return _is_constant; }

	    //dbg function
        bool validate_probability()
        {
            std::vector<int> keys;
            double probability = 0.0;
            double goal = 100.0;
            // Get the list of keys
            for(auto it : _buckets )
            {
                keys.push_back(it.first);

            }
            // Use the list of keys to sum the values
            for (int i = 0; i != keys.size(); i++)
            {
                probability += _buckets[keys[i]];
            }
            // The or second comparisson is used handle the double inprecision
            return (goal == probability ||
            std::abs(goal - probability) < std::abs(std::min(goal, probability))*katch::EPSILON);
        }

        uint32_t get_min_cost()
        {
            // map containers keep their elements ordered at all times, begin points to the element that goes first
            return _buckets.begin()->first;
        }

        uint32_t get_max_cost()
        {
            // ka man ikke bare returnere det sidste element?
            uint32_t max = 0;
            for (auto it =_buckets.begin(); it!=_buckets.end(); ++it)
            {
                assert(it->first>=max);
                max = it->first;
            }
            return max;
        }

        Histogram merge(const Histogram rhs){
            uint32_t rhs_number_of_measurements = rhs.get_n_measurements();
            uint32_t lhs_number_of_measurements = this->get_n_measurements();
            if(!util::time_equal(this->get_tp_start(), rhs.get_tp_start())){
                if(util::time_before(rhs.get_tp_start(), this->get_tp_start())){
                    this->set_tp_start(rhs.get_tp_start());
                }
                if(util::time_before(this->get_tp_end(), rhs.get_tp_end())){
                    this->set_tp_end(rhs.get_tp_end());
                }
            }
            auto bckts = this->get_buckets();
            for (auto it : rhs.get_buckets())
            {
                auto i = bckts.find(it.first);
                if(i != this->get_buckets().end())
                {
                    i->second = i->second*lhs_number_of_measurements + it.second*rhs_number_of_measurements;
                }
                else
                {
                    this->get_buckets()[it.first] = it.second*rhs_number_of_measurements;
                }
            }
            this->set_n_measurements(lhs_number_of_measurements + rhs_number_of_measurements);
            for (auto b : get_buckets() )
            {
                b.second = b.second*100 / this->get_n_measurements();

            }
            assert(this->validate_probability());
            return *this;
        }
        // checks dominance
        // If you want to check if the fields are equal, use is_equal member function
        Histogram operator+(const Histogram&);
        Histogram& operator*(double multiplier) const;
    };

    const Histogram Histogram::INFTY = Histogram();


    Histogram& Histogram::operator*(double multiplier) const
    {
        auto buckets = this->get_buckets();
        std::map<uint32_t, double> buckets_with_confidence;

        for (auto b = buckets.begin(); b != buckets.end(); ++b)
        {
            buckets_with_confidence[b->first] = b->second * multiplier;
        }
        auto result = new Histogram(this->get_tp_start(), this->get_tp_start(), this->get_n_measurements(), buckets_with_confidence);
        return *result;
    }

    Histogram Histogram::operator+(const Histogram& rhs){
        return this->merge(rhs);
    }
}

#endif /* 'KATCH_HISTOGRAM */
