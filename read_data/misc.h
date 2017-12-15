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

#define KATCH_IMPLIES(a, b) ((!(a)) || (b))

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


using TimePoint = std::chrono::high_resolution_clock::time_point;

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
