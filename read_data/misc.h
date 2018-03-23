//TODO: Fjern katch ting
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

enum GraphType {aalborg=1, nort_jutand, denmark};
enum TimeType {alldata=1, peak, days};

/**
 * Here are the different peak intervals that are used in peak_vs_off_peak
 */
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

/**
 * Compare ID against enum to determine the graph type
 * @param type an ID from the input file in the range [1-3]
 * @return the result will be used as a prefix for the output files
 */
std::string get_location_from_graph_type(int type) {
    if (type == aalborg)
        return "AAl";
    else if (type == nort_jutand)
        return "NJ";
    else if (type == denmark)
        return "DK";
    else return "UNKNOWN";
}

/**
 * Test if two struct tm is equal. Date is ignored
 * @param lhs
 * @param rhs
 * @return True if equal, false if not
 */
bool time_equal(const time& lhs, const time& rhs) {
    return lhs.tm_sec == rhs.tm_sec &&
           lhs.tm_min == rhs.tm_min &&
           lhs.tm_hour == rhs.tm_hour;
}

/**
 * Test if one date is before another. Date is ignored
 * @param lhs
 * @param rhs
 * @return True if lhs is before rhs, false if not
 */
bool time_before(const time& lhs, const time& rhs) {
    if(lhs.tm_hour < rhs.tm_hour)
        return true;
    if(lhs.tm_hour == rhs.tm_hour && lhs.tm_min < rhs.tm_min)
        return true;
    return lhs.tm_hour == rhs.tm_hour && lhs.tm_min == rhs.tm_min && lhs.tm_sec < rhs.tm_sec;
}

extern "C" char* strptime(const char* s,
                          const char* f,
                          struct tm* tm) {
    /* KaTCH -- Karlsruhe Time-Dependent Contraction Hierarchies
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
     */

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
