#include <iostream>
#include "median_filter.h"

using namespace std;

int main() 
{
    dtype image_in[M*N] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};

    dtype image_out[M*N];

    median_filter(image_in, image_out);

    for (int i = 0; i < M * N; i++) {
		cout << image_out[i] << " ";
	}

}
