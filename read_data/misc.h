/*
 * katch/util/misc.h
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

#ifndef KATCH_MISC_H_
#define KATCH_MISC_H_

#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <chrono>
#include <functional>
#include <limits>
#include <vector>
#include <tuple>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "basic.h"

#define AN_ENTIRE_DAY 86400

#define KATCH_IMPLIES(a, b) ((!(a)) || (b))
#define KATCH_EQUIV(a, b) (KATCH_IMPLIES((a),(b)), KATCH_IMPLIES((b),(a)))

#define KATCH_BE_VERBOSE true
#define KATCH_ERROR_VERBOSE true
#define KATCH_STATUS_VERBOSE true
#define KATCH_WARNING_VERBOSE true

#define KATCH_STATE(s) if (KATCH_BE_VERBOSE) std::cout << "STATE [" << __FILE__ << ":" << __LINE__  << "] " << s << std::flush
#define KATCH_STATUS(s) if (KATCH_STATUS_VERBOSE) std::cout << "STATUS [" << __FILE__ << ":" << __LINE__  << "] " << s << std::flush
#define KATCH_WARNING(s) if (KATCH_WARNING_VERBOSE) std::cerr << "WARNING [" << __FILE__ << ":" << __LINE__  << "] " << s << std::flush
#define KATCH_ERROR(s) if (KATCH_ERROR_VERBOSE) std::cerr << "ERROR [" << __FILE__ << ":" << __LINE__  << "] " << s << std::flush

#define KATCH_CONTINUE_STATUS(s) if (KATCH_STATUS_VERBOSE) std::cout << s << std::flush
#define KATCH_CONTINUE_ERROR(s) if (KATCH_ERROR_VERBOSE) std::cerr << s << std::flush
#define KATCH_CONTINUE_WARNING(s) if (KATCH_WARNING_VERBOSE) std::cerr << s << std::flush

enum GraphType {aalborg=1, nort_jutand, denmark};
enum TimeType {alldata=1, peak, days};

             // se mi hr md mo yr wd yd
struct tm peak1{0, 0, 0, 1, 0, 0, 1, 0};
struct tm peak2{0, 0, 7, 1, 0, 0, 1, 0};
struct tm peak3{0, 30, 8, 1, 0, 0, 1, 0};
struct tm peak4{0, 0, 15, 1, 0, 0, 1, 0};
struct tm peak5{0, 0, 17, 1, 0, 0, 1, 0};
struct tm day_end{0, 0, 23, 1, 0, 0, 1, 0};
struct tm day_start{0, 0, 0, 1, 0, 0, 1, 0};

namespace katch
{
namespace util
{
using time = struct tm;

    time MAX_TIME = {
        60, // sec
        59, // min
        23, // hour
        1,  // day of month
        0,  // mon
        0,  // year
        0,  // weekday
        0   // day of year
};

    time MIN_TIME = {
            0,  // sec
            0,  // min
            0,  // hour
            1,  // day of month
            0,  // mon
            0,  // year
            0,  // weekday
            0   // day of year
    };


std::string get_location_fromgraphtype(int type) {
    if (type == aalborg)
        return "AAl";
    else if (type == nort_jutand)
        return "NJ";
    else if (type == denmark)
        return "DK";
    else return "UNKNOWN";
}

// Check whether a range (of "less" and "equal_to" comparable elements) contains duplicates
template
<
    typename Iterator,
    typename LtFn = std::less<typename std::iterator_traits<Iterator>::value_type>,
    typename EqFn = std::equal_to<typename std::iterator_traits<Iterator>::value_type>
>
bool has_duplicates
(
        const Iterator& begin,
        const Iterator& end,
        const LtFn& lt = std::less<typename std::iterator_traits<Iterator>::value_type>(),
        const EqFn& eq = std::equal_to<typename std::iterator_traits<Iterator>::value_type>()
)
{
    std::vector<typename std::iterator_traits<Iterator>::value_type> vec(begin, end);

    std::sort(vec.begin(), vec.end(), lt);
    auto has_duplicate_result = std::adjacent_find(vec.begin(), vec.end(), eq); // pointer to duplicate element. If no duplicates, equals end().
    return has_duplicate_result != vec.end();
}

// Check whether a vector (of "less" and "equal_to" comparable elements) contains duplicates
template <typename T>
bool has_duplicates(const std::vector<T>& vec)
{
    return has_duplicates(vec.begin(), vec.end());
}


// Remove duplicates from a vector of "less" and "equal_to" comparable elements
template <typename T, typename LtFn = std::less<T>, typename EqFn = std::equal_to<T>>
void remove_duplicates(std::vector<T>& vec, const LtFn& lt = std::less<T>(), const EqFn& eq = std::equal_to<T>())
{
    std::sort(vec.begin(), vec.end(), lt);
    auto new_end = std::unique(vec.begin(), vec.end(), eq);

    vec.resize( std::distance(vec.begin(), new_end) );
    assert( ! has_duplicates(vec) );
}

template <typename Graph, typename Iterator>
std::vector<NodeIterator> neighborhood(const Graph& graph, Iterator begin, Iterator end)
{
    std::vector<NodeIterator> result;

    for ( auto it = begin ; it != end ; ++it )
    {
        const auto& x = *it;
        std::vector<NodeIterator> neighbors_of_x;

        for ( auto e = graph.out_edges_begin(x) ; e != graph.out_edges_end(x) ; ++e )
            neighbors_of_x.push_back(graph.get_target(e));

        for ( auto e = graph.in_edges_begin(x) ; e != graph.in_edges_end(x) ; ++e )
            neighbors_of_x.push_back(graph.get_source(e));

        remove_duplicates(neighbors_of_x);
        result.insert(result.end(), neighbors_of_x.begin(), neighbors_of_x.end());
    }

    return result;
}

// Compute the position of the most significant bit (where the right most position is 0)
inline uint32_t msb(const uint32_t x)
{
    int digit = std::numeric_limits<uint32_t>::digits - 1; //Number of digits in uint32_t.
    while( (x & (1 << digit)) == 0 ) --digit;

    assert( digit >= 0 );
    return digit;
}

// Compute x mod m
inline double modulo(const double x, const double m)
{
    if (x >= 0)
    {
        if ( x < m ) return x;
        if ( x < m+m ) return x - m;
    }

    double result = fmod(x, m);
    if (result < 0) result += m;

    assert( 0 <= result );
    assert( result < m );
    assert( fabs(result - (x - m * floor(x/m))) < 0.000001 );

    return result;
}

// compute x mod m
inline int modulo(const int x, const int m)
{
    int result = x % m;
    if (result < 0) result += m;

    assert(result >= 0);
    assert( result == (x - m * floor((double)x/(double)m)) );

    return result;
}


double random(const double a, const double b)
{
    assert( a < b );
    double result =  (double(rand()) / double(RAND_MAX)) * (b - a) + a;

    assert( a <= result + 0.000001 );
    assert( result <= b + 0.000001 );
    return result;
}

using TimePoint = std::chrono::high_resolution_clock::time_point;

inline TimePoint time_stamp()
{
    return std::chrono::high_resolution_clock::now();
}

inline double get_duration_in_seconds(const TimePoint& from, const TimePoint& to)
{
    assert( from <= to );
    return std::chrono::duration_cast<std::chrono::duration<double>>(to - from).count();
}

time create_time_struct_from_seconds(uint16_t x) // x = total number of seconds
{
    int seconds, minutes, hours;
    if (seconds == AN_ENTIRE_DAY)
    {
        seconds = 59;
        minutes = 59;
        hours = 23;
    }
    else if (x > AN_ENTIRE_DAY)
    {
        x += -AN_ENTIRE_DAY;
        seconds = x % 60;
        minutes = (x - seconds) / 60 % 60;
        hours = (x - seconds - minutes * 60) / 60 / 60;
    }
    else
    {
        seconds = x % 60;
        minutes = (x - seconds) / 60 % 60;
        hours = (x - seconds - minutes * 60) / 60 / 60;
    }
    time result = {
        seconds,
        minutes,
        hours,
    };
    return result;
}

int32_t get_total_seconds(time t)
{
        return t.tm_hour*60*60 + t.tm_min*60 + t.tm_sec;
}


// lhs and rhs are equal - date is ignored
bool time_equal(const time& lhs, const time& rhs)
{
    // Return whether or not the two time structs are equal
    return lhs.tm_sec == rhs.tm_sec &&
           lhs.tm_min == rhs.tm_min &&
           lhs.tm_hour == rhs.tm_hour;
}

// lhs is before rhs
bool time_before(const time& lhs, const time& rhs)
{
    if(lhs.tm_hour < rhs.tm_hour)
        return true;
    if(lhs.tm_hour == rhs.tm_hour && lhs.tm_min < rhs.tm_min)
        return true;
    return lhs.tm_hour == rhs.tm_hour && lhs.tm_min == rhs.tm_min && lhs.tm_sec < rhs.tm_sec;
}

// lhs is after rhs
bool time_after(const time& lhs, const time& rhs)
{
    return !time_before(lhs, rhs) && !time_equal(lhs, rhs);
}

        extern "C" char* strptime(const char* s,
                                  const char* f,
                                  struct tm* tm) {
            // Isn't the C++ standard lib nice? std::get_time is defined such that its
            // format parameters are the exact same as strptime. Of course, we have to
            // create a string stream first, and imbue it with the current C locale, and
            // we also have to make sure we return the right things if it fails, or
            // if it succeeds, but this is still far simpler an implementation than any
            // of the versions in any of the C standard libraries.
            std::istringstream input(s);
            input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
            input >> std::get_time(tm, f);
            if (input.fail()) {
                return nullptr;
            }
            return (char*)(s + input.tellg());
        }

} /* namespace util */
} /* namespace katch */

#endif /* KATCH_MISC_H_ */
