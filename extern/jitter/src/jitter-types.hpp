//Copyright 2014, Intermodalics BVBA


#ifndef __JITTER_TYPES_HPP__
#define __JITTER_TYPES_HPP__

namespace jitter{
struct stats {
    double current;
    double max;
    double min;
    double avg;
    double nr_of_samples;
};
}
#endif
