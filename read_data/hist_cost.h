//
// Created by simon on 3/28/17.
//

#ifndef KATCH_HIST_COST_H
#define KATCH_HIST_COST_H


#include <vector>
#include "histogram.h"

namespace katch {

    using Histogram = katch::Histogram;
    using time = struct tm;

    class HistCost {

    private:
        std::vector<Histogram> _histograms;

    public:
        // empty
        HistCost() = default;
        ~HistCost() = default;

        // vector of histograms
        HistCost(std::vector<Histogram> hist):_histograms(hist){
            uint32_t min = INT32_MAX, max = 0;
            for (auto hist : _histograms) {
                if (hist.get_max_cost() > max) {
                    max = hist.get_max_cost();
                }
                if (hist.get_min_cost() > min) {
                    max = hist.get_min_cost();
                }
            }
        };

        // single histogram
        HistCost(Histogram hist){
            std::vector<Histogram> hists = {hist};
            _histograms = hists;
            uint32_t min = INT32_MAX, max = 0;
        };

        std::vector<Histogram> get_histograms() const { return _histograms; }
    };
}

#endif //KATCH_HIST_COST_H
