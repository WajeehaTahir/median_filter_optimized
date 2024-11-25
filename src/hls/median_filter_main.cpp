#include <iostream>
#include "median_filter.h"

using namespace std;

int main() 
{
    /*dtype image_in[M*N] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25}; */
    
    // dtype image_in[M*N] = {0, 2, 1 ,1 ,1 , 2, 3 ,3, 0, 1, 0, 1, 1, 2, 1, 4};

    dtype image_in[M*N] = 
    {
        1, 4, 0, 1, 3, 1,
        2, 2, 4, 2, 2, 3, 
        1, 0, 1, 0, 1, 0,
        1, 2, 1, 0, 2, 2, 
        2, 5, 3, 1, 2, 5,
        1, 1, 4, 2, 3, 0
    };

    dtype image_out[M*N];

    median_filter(image_in, image_out);

    for (int i = 0; i < M * N; i++) {
		cout << image_out[i] << " ";
	}

}
