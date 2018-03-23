/*
 * katch/datastr/graph/iterators.h
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

#ifndef KATCH_ITERATORS_H_
#define KATCH_ITERATORS_H_

#include <cstdint>
#include <limits>
#include <time.h>

#include <boost/serialization/strong_typedef.hpp>

namespace katch
{

using NodeIterator = uint32_t;// Creates an alias for a type. 

BOOST_STRONG_TYPEDEF(uint32_t, FwEdgeIterator); // Creates an alias for a type. Here it's an alias for uint32 named FwEdgeIterator.
BOOST_STRONG_TYPEDEF(uint32_t, BwEdgeIterator);
BOOST_STRONG_TYPEDEF(uint32_t, EdgeIterator);

const NodeIterator INVALID_NODE_ITERATOR(std::numeric_limits<NodeIterator>::max());
const FwEdgeIterator INVALID_FW_EDGE_ITERATOR(std::numeric_limits<uint32_t>::max());
const BwEdgeIterator INVALID_BW_EDGE_ITERATOR(std::numeric_limits<uint32_t>::max());
const EdgeIterator INVALID_EDGE_ITERATOR(std::numeric_limits<uint32_t>::max());

struct MiddleCostNodeDescr
{
    using time = struct tm;
    time _time;
    NodeIterator _middle_node;

    MiddleCostNodeDescr(const time& time, const NodeIterator& middle_node)
    : _time(time), _middle_node(middle_node)
    {}

    friend bool operator== (const MiddleCostNodeDescr& lhs, const MiddleCostNodeDescr& rhs)
    {
        return lhs._time.tm_hour == rhs._time.tm_hour &&
        lhs._time.tm_min == rhs._time.tm_min &&
        lhs._time.tm_sec == rhs._time.tm_sec;
    }

    friend bool operator!= (const MiddleCostNodeDescr& lhs, const MiddleCostNodeDescr& rhs)
    {
        return ! (lhs == rhs);
    }

    int get_total_seconds() const
    {
        return _time.tm_sec + _time.tm_min * 60 + _time.tm_hour * 60 * 60;
    }
};

}

#endif /* KATCH_ITERATORS_H_ */
