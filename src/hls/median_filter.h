#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include "hls_stream.h"

#define M (5)
#define N (8)
#define F (3)

typedef int dtype;

void median_filter(hls::stream <dtype> &image_in, hls::stream <dtype> &image_out);

#endif
