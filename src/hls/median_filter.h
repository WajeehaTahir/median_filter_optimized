#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#define M (5)
#define N (5)
#define F (3)

typedef int dtype;

void median_filter(dtype *image_in, dtype *image_out);

#endif
