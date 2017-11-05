//
// Created by Simon on 25/04/2017.
//
#ifndef KATCH_SHORTCUT_H
#define KATCH_SHORTCUT_H

#include <basic.h>
#include "hist_cost.h"

namespace  katch {

    struct Shortcut {

        HistCost* cost;
        std::vector<NodeIterator> nodeList;
        // TODO: consider ForwardIterator instead.
        /*~Shortcut(){
            delete cost;
        }*/
    };
}

#endif //KATCH_SHORTCUT_H
