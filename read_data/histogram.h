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


namespace katch
{

class Histogram
{
    using time = struct tm;
private:

    time _tp_start;
    time _tp_end;
    uint32_t _n_measurements;
    bool _is_constant = false;  // if _n_measurements = 0, there will only be one bucket with the cost equal to the speed limit times length
    std::map<uint32_t,double> _buckets;  // cost, probability


public:
    static const Histogram INFTY;

    enum class RelativePosition { UNDEFINED = 0, F_ABOVE = 1, G_ABOVE = 2 };

#pragma region Constructors

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
#pragma endregion

#pragma region getters/setters
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

        /*
        bool is_equal(Histogram h)
        {
            if (!util::time_equal(this->get_tp_start(), h.get_tp_start())) return false;
            if (!util::time_equal(this->get_tp_end(), h.get_tp_end())) return false;
            if (this->get_n_measurements() != h.get_n_measurements()) return false;
            if (this->is_constant() != h.is_constant()) return false;

            auto lhs_buckets = this->get_buckets();
            auto rhs_buckets = h.get_buckets();
            if (lhs_buckets.size() != rhs_buckets.size()) return false;
            for (auto it = lhs_buckets.begin(); it != lhs_buckets.end(); ++it)
            {
                if (lhs_buckets[it->first] != rhs_buckets[it->first]) return false;
            }

            return true;
        }*/
        #pragma endregion

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

        double get_cpi(uint32_t x_value)  // cpi = Cumulative Probability Intervals
        {
            assert(x_value > 0);
            double probability = get_probability(x_value);
            if (probability == 0)
            {
                std::map<uint32_t, double>::iterator itlow, itup;
                itup = _buckets.upper_bound(x_value);
                itlow = _buckets.lower_bound(x_value);
                if (itlow != _buckets.begin())
                {
                    itlow--;
                }
                if (itup == _buckets.end())
                {
                    itup--;
                }

                uint32_t upx = itup->first, lwx = itlow->first;
                double upy = itup->second, lwy = itlow->second;
                auto slope = (upy - lwy) / (upx - lwx);
                uint32_t dist = x_value - lwx;
                assert(dist > 0);

                probability = itlow->second + dist * slope;
                assert(probability > 0);
            }
            return probability;
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
        bool operator==(const Histogram&) const;
        bool operator<(const Histogram&) const;
        bool operator>(const Histogram&) const;
        Histogram operator+(const Histogram&);
        Histogram& operator*(double multiplier) const;
};

    const Histogram Histogram::INFTY = Histogram();


    // Helper function for stochastic_dominance
    // Update accumulation to cost
    // Useful when you don't know which index we need to stop at
    double update_accumulation_to_cost(std::vector<uint32_t > keys, Histogram const * const histogram, int const stop_cost)
    {
        double accumulation = 0.0;
        uint32_t it = 0;
        do
        {
            accumulation += histogram->get_probability(keys[it]);
            if (accumulation == 100.0) {
                break;
            }
                it++;
            } while (keys[it] <= stop_cost);

            return accumulation;
        }

    // Helper function for stochastic_dominance
    // Update accumulation to index
    double update_accumulation(std::vector<uint32_t > keys, Histogram const * const histogram, unsigned long stop)
    {
        double accumulation = 0.0;
        if (stop == 0) {
            accumulation += histogram->get_probability(keys[0]);
        }
        else
        {
            for (auto i = 0; i <= stop; i++) {
                accumulation += histogram->get_probability(keys[i]);
            }
        }
        return accumulation;
    }

    // Helper function for stochastic_dominance
    // Accumulate(total sum of) the probability of the histograms up to a certain cost
    std::pair<bool, bool> accumulator(unsigned long index, Histogram const * const left, Histogram const * const right,
                                      std::vector<uint32_t > leftkeys, std::vector<uint32_t > rightkeys)
    {
        // These booleans will be used to determine the dominance when the function returns
        bool left_dominance = false, right_dominance = false;
        double left_accumulation = update_accumulation(leftkeys, left, index), // left_acc is 0 at index = 0
               right_accumulation = 0.0;

        // if right has an entry for the same cost left has, the probability will
        if (right->get_probability(leftkeys[index]) != -1)
        {
            // The case where both buckets have the same cost at the same index (rare)
            if (rightkeys[index] == leftkeys[index])
                right_accumulation = update_accumulation(rightkeys, right, index);
            // The case where right has the same cost, but at a different index
            else
                right_accumulation = update_accumulation_to_cost(rightkeys, right, leftkeys[index]);
        }
        // The case where right does not have a cost that left does
        else
        {
            // Find the index of right that is just before the cost of left (4 vs 5 OK, 3 vs 5 OK, 6 vs 5 NOT OK)
            // It can never have the same cost, as that will lead to the cases above where right->get_probability(leftkeys[index]) != -1
            for (auto j = rightkeys.size(); j != 0 ; j--)
                {
                    if(rightkeys[j-1] <= leftkeys[index])
                    {
                        // The right cost is now less or equal to the left side
                        assert( j - 1 >= 0);
                        right_accumulation = update_accumulation(rightkeys, right, j - 1);
                        break;
                    }
                    // If we reach the first key in right, it means that right's smallest cost is larger
                    // than what left is at
                    else if (j == 1)
                    {
                        if (rightkeys[0] > leftkeys[index])
                            right_accumulation = 0.0;
                        else
                            right_accumulation = 100.0;
                    }
                }
        }
        if (left_accumulation > right_accumulation)
            left_dominance = true;
        else if (left_accumulation < right_accumulation)
            right_dominance = true;

        std::pair<bool, bool> result(left_dominance, right_dominance);
        return result;
    }

    // Calculate whether or not one histogram dominates another - left vs right
    enum dominance { left_dom, right_dom, no_dom };  // The enum is used as the return type for the function below
    dominance stochastic_dominance(Histogram const * const left, Histogram const * const right)
    {
        // list keys to the buckets
        std::vector<uint32_t > leftkeys;
        std::vector<uint32_t > rightkeys;
        // the actual buckets
        std::map<uint32_t,double> left_buckets = left->get_buckets();
        std::map<uint32_t,double> right_buckets = right->get_buckets();

        // Trivial check
        if (left_buckets.size() == 0 && right_buckets.size() == 0)
            return no_dom;
        else if (left_buckets.size() == 0)
            return right_dom;
        else if (right_buckets.size() == 0)
            return left_dom;

        // get all left keys and
        for(std::map<uint32_t, double>::iterator it = left_buckets.begin(); it != left_buckets.end(); ++it)
            leftkeys.push_back(it->first);
        // get all right keys
        for(std::map<uint32_t, double>::iterator it = right_buckets.begin(); it != right_buckets.end(); ++it)
            rightkeys.push_back(it->first);

        // These booleans will be used to determine the dominance when the function returns
        bool left_dominance = false, right_dominance = false;
        std::pair<bool, bool> results;

        // compare left with right
        for (unsigned long i = 0; i != leftkeys.size(); i++)
        {
            results = accumulator(i, left, right, leftkeys, rightkeys);  // returns a pair of booleans
            if (results.first)  // left dominance
                left_dominance = true;  // observe that the result will never change the booleans to false
            else if (results.second)  // right dominance
                right_dominance = true;
        }

        // compare right with left
        for (unsigned long i = 0; i != rightkeys.size(); i++)
        {
            results = accumulator(i, right, left, rightkeys, leftkeys);
            if (results.first)
                right_dominance = true;
            else if (results.second)
                left_dominance = true;
        }

        // Both histograms have now been compared with each other, both ways
        // The return statements utilize the enum that is defined at the beginning of this function
        if ((left_dominance && right_dominance) || (!left_dominance && !right_dominance))
            return no_dom;  // No dominance
        else if (left_dominance)
            return left_dom;
        else
            return right_dom;
    }


    bool Histogram::operator==(const Histogram &rhs) const
    {
        return stochastic_dominance(this, &rhs) == no_dom;
    }

    bool Histogram::operator<(const Histogram& rhs) const
    {
        return stochastic_dominance(this, &rhs) == right_dom;

    }

    bool Histogram::operator>(const Histogram& rhs) const
    {
        return stochastic_dominance(this, &rhs) == left_dom;
    }

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
