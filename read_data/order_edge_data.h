/*
 * katch/datastr/graph/order_edge_data.h
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

#ifndef KATCH_ORDER_EDGE_DATA_H_
#define KATCH_ORDER_EDGE_DATA_H_


#include <assert.h>
#include <limits>
#include <memory>
#include <utility>
#include <time.h>
#include <hist_cost.h>
#include <shortcut.h>

#include "basic.h"
#include "misc.h"

namespace katch
{

class OrderEdgeData
{

public:

    using time = struct tm;
    struct UndercutDescriptor
    {
        bool _f_undercuts_strictly;
        bool _g_undercuts_strictly;
    };

private:

    // number of original edges represented by this (shortcut) edge
    unsigned int _n_original_edges;

    // size of of shortcut descriptor
    unsigned int _shortcut_descr_size: 31;

    // tells whether this edge has constant cost
    bool _is_constant : 1;

    HistCost* hist_cost = new HistCost();

    std::vector<Shortcut>* shortcuts = new std::vector<Shortcut>();


public:
    OrderEdgeData(const OrderEdgeData&) = delete;
    OrderEdgeData& operator= (const OrderEdgeData&) = delete;

    OrderEdgeData(OrderEdgeData&& order_edge_data)
    : _n_original_edges(order_edge_data._n_original_edges),
      _shortcut_descr_size(order_edge_data._shortcut_descr_size)
    {
        if ( this != &order_edge_data )
        {
            _n_original_edges = order_edge_data._n_original_edges;
            _shortcut_descr_size = order_edge_data._shortcut_descr_size;
            _is_constant = order_edge_data._is_constant;

            hist_cost = order_edge_data.get_cost_pointer();
            shortcuts = order_edge_data.get_shortcuts_pointer();

            order_edge_data.reset();
            order_edge_data._is_constant = false;

        }
    }
    // move
    OrderEdgeData& operator= (OrderEdgeData&& order_edge_data)
    {
        if ( this != &order_edge_data )
        {
            set_cost(static_cast<HistCost &&>(order_edge_data.get_cost()));
            set_up(order_edge_data);

            _n_original_edges = order_edge_data._n_original_edges;
            _shortcut_descr_size = order_edge_data._shortcut_descr_size;
            _is_constant = order_edge_data._is_constant;

            // Ensure deletion of resources.
            reset();
            hist_cost = order_edge_data.get_cost_pointer();
            shortcuts = order_edge_data.get_shortcuts_pointer();
            order_edge_data.reset();

            order_edge_data._is_constant = false;


        }

        return *this;
    }

    OrderEdgeData()
    : _n_original_edges(1),
      _shortcut_descr_size(0),
      _is_constant(false)
    {}

    ~OrderEdgeData()
    {

        if (hist_cost != nullptr) delete hist_cost;
        if (shortcuts != nullptr) delete shortcuts;
    }

    void reset(){
        hist_cost = nullptr;
        shortcuts = nullptr;
    }

    std::vector<Shortcut>* get_shortcuts_pointer(){
        return shortcuts;
    }
    std::vector<Shortcut> get_shortcuts(){
        return *shortcuts;
    }

    std::vector<Shortcut> const get_shortcuts()const {
        return *shortcuts;
    }

    void set_cost(const HistCost& cst)
    {
        if (hist_cost != nullptr) delete hist_cost;
        hist_cost = new HistCost(cst);
        _is_constant = hist_cost->is_constant();
    }

    void set_cost(HistCost &&cst)
    {
        if (hist_cost != nullptr) delete hist_cost;
        hist_cost = new HistCost(cst);
        _is_constant = hist_cost->is_constant();
    }

    void set_cost(HistCost* cst)
    {
        if (hist_cost != nullptr) delete hist_cost;
        hist_cost = cst;
        cst = nullptr;
        _is_constant = hist_cost->is_constant();
    }


    HistCost& get_cost(){
        return *hist_cost;
    }

    const HistCost& get_const_cost() const{
        return *hist_cost;
    }

    HistCost* get_cost_pointer(){
        return hist_cost;
    }

    const HistCost* get_const_cost_pointer()const {
        return hist_cost;
    }

    template <typename EdgeCost>
    void set_up(EdgeCost&& edge_info) noexcept
    {
        //set_ttf( std::move(edge_info.get_ttf()) );
        set_cost(edge_info.get_cost());
    }


    void set_n_original_edges(const unsigned int n_original_edges) noexcept
    {
        _n_original_edges = n_original_edges;
    }

    unsigned int get_n_original_edges() const noexcept
    {
        return _n_original_edges;
    }

    bool never_represents_a_shortcut() const noexcept
    {
        return (*shortcuts).size() == 0;

    }

    size_t get_n_shortcut_nodes() const
    {
        return (*shortcuts).size();
    }

    uint32_t get_shortcut_descr_size() const
    {
        return _shortcut_descr_size;
    }

    bool is_constant() const {
        return _is_constant;
    }

    void add_shortcut(Shortcut&& sc){
        (*shortcuts).push_back(std::move(sc));
    }

    void add_shortcut(Shortcut& sc){
        (*shortcuts).push_back(sc);
    }

    template <typename Iterator>
    void add_shortcuts(Iterator begin, Iterator end)
    {
        if(end == begin()){
            return;
        }
        auto size = (*shortcuts).size();
        (*shortcuts).insert((*shortcuts).end(), begin, end);

        assert((*shortcuts).size() == size + (end-begin));

    }

};

}

#endif /* KATCH_ORDER_EDGE_DATA_H_ */
