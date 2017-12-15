#ifndef KATCH_EDGECOST
#define KATCH_EDGECOST

#include <iostream>
#include <assert.h>
#include <stdint.h>
#include <cmath>
#include <limits>
#include <vector>
#include <deque>
#include <time.h>
#include "misc.h"
#include "histogram.h"

namespace katch
{

class Edge
{
public:

    using Histogram = katch::Histogram;

private:

    uint32_t _edge_id;
    size_t _source_node; // Edges are directed
    size_t _target_node;
    unsigned int _n_original_edges;

    HistCost* cost;

public:
    void operator=( Edge&& other){

        _edge_id = other._edge_id;
        _source_node = other.get_source();
        _target_node = other.get_target();
        _n_original_edges = other.get_n_original_edges();
        cost = other.get_cost_pointer();
        other.reset();
    }

    // Constructors
    Edge(Edge&& other)
    : _edge_id(other.get_edge_id()), _source_node(other.get_source()), 
    _target_node(other.get_target())
    {
        cost = other.get_cost_pointer();
        other.reset();
    }

    // Full constructor
    Edge(uint32_t edge_id, uint32_t start_node, uint32_t target_node, const std::vector<Histogram>& histograms)
    : _edge_id(edge_id), _source_node(start_node), _target_node(target_node)
    {
        cost = new HistCost(histograms);
    }

    // edge id, start, target
    Edge(uint32_t edge_id, uint32_t start_node, uint32_t target_node)
    : _edge_id(edge_id), _source_node(start_node), _target_node(target_node) /*_is_constant(_is_constant),*/
    {
        cost = new HistCost();
    }

    Edge(uint32_t u, uint32_t v, uint32_t x, uint32_t shortcut_complexity, unsigned int n_original_edges){

        _source_node = u, _target_node = v,
        _n_original_edges = n_original_edges;
        //_middle_node_data.push_back(MiddleCostNodeDescr(tm {0,0,0}, x));
        cost = new HistCost();
    }

    // Empty constructor
    Edge() = default;

    ~Edge() = default;

    unsigned int get_n_original_edges()const {
        return _n_original_edges;
    }

    //  edge id
    void set_edge_id(uint32_t edge_id){ _edge_id = edge_id; }
    uint32_t get_edge_id()const { return _edge_id; }

    // source node
    void set_source(uint32_t start_node){ _source_node = start_node; }
    size_t get_source() const { return _source_node; }

    // target node
    void set_target(uint32_t target_node){ _target_node = target_node; }
    size_t get_target() const { return _target_node; }

    // cost
    void set_cost(HistCost && cst){
        if(cost != nullptr) delete cost;
        cost = new HistCost(cst);
    }

    HistCost& get_cost() const {return *cost;}

    HistCost* get_cost_pointer(){ return cost;}

    void reset(){
        cost = nullptr;
    }

};
}

#endif /* 'KATCH_EDGECOST */
