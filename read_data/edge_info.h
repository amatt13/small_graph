/*
 * katch/io/edge_info.h
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

#ifndef KATCH_EDGE_INFO_H_
#define KATCH_EDGE_INFO_H_

#include <cassert>
#include <limits>
#include <vector>
#include <hist_format.h>
#include <hist_cost.h>

#include "basic.h"

namespace katch
{
class EdgeInfo
{

private:

    size_t _src;
    size_t _tgt;
    katch::HistCost _cost;
    std::vector<MiddleCostNodeDescr> _middle_node_data;
    unsigned int _complexity = 0;
    unsigned int _n_original_edges;

public:

    unsigned int get_n_original_edges(){
        return _n_original_edges;
    }

    size_t get_source() const { return _src; }

    size_t get_target() const { return _tgt; }

    void set_cost(HistCost&& cst) { _cost = std::move(cst); }
    const HistCost& get_ttf() const { return _cost; }
    HistCost& get_cost() { return _cost; }

    int complexity(){
        if (_complexity != 0){
            return _complexity;
        }
        return _cost.size();
    }
    void set_complexity(int val){
        _complexity = val;
    }

    void set_middle_node_data(std::vector<MiddleCostNodeDescr>&& middle_node_data)
    {
        _middle_node_data = std::move(middle_node_data);
    }

    const std::vector<MiddleCostNodeDescr>& get_middle_node_data() const
    {
        return _middle_node_data;
    }

#pragma region Constructors
    EdgeInfo()
            : _src(std::numeric_limits<size_t>::max()),
              _tgt(std::numeric_limits<size_t>::max()),
            //_forward(false),
            //_backward(false),
              _cost(),
              _middle_node_data()
    {}

    EdgeInfo(const NodeIterator& src, const NodeIterator& tgt,
                    const NodeIterator& middle_node, const unsigned int& complexity, const unsigned int& n_original_edges)
            : _src(src), _tgt(tgt), _cost(),
              _n_original_edges(n_original_edges)
    {
        set_complexity(complexity);
        _middle_node_data.emplace_back(MiddleCostNodeDescr(tm {0,0,0}, middle_node)); // TODO: Arbitrary time, does not matter (?)
    }

#pragma endregion
};
}

#endif /* KATCH_EDGE_INFO_H_ */
