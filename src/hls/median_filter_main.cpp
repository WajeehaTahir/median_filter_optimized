#include <iostream>
#include "median_filter.h"

using namespace std;

dtype* sort_ascending_sw(dtype * a, int n)
{
    for(int i=0; i<n; i++)
    {
        for(int j=i+1; j<n; j++) 
        { 
            if(a[i]>a[j])
            {
                int temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
    }

    return a;
}

dtype median_sw(dtype* window, int n)
{
    return sort_ascending_sw(window, n)[n/2];
}

void median_filter_sw(dtype *image_in, dtype *image_out)
{
    dtype image[M][N], window[F*F], filtered_image[M][N];

	load_image: for (int i = 0; i < M * N; i++) {
		image[i / N][i % N] = image_in[i];
	}

    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            int count = 0;
            for (int k = -F/2; k <= F/2; k++)
            {
                for (int l = -F/2; l <= F/2; l++)
                {
                    if ((i+k) < 0 || (j+l) < 0 || (i+k) >= M || (j+l) >= N)
                    {
                        window[count] = 0;
                    }
                    else {
                        window[count] = image[i+k][j+l];
                    }
                    
                    count++;
                }
            }

            filtered_image[i][j] = median_sw(window, F*F);
        }
    }

    save_output: for (int i = 0; i < M * N; i++) {
		image_out[i] = filtered_image[i / N][i % N];
	}

}

int main() 
{
    dtype image_in[M*N] = {
        1, 2, 3, 4, 5, 6, 7, 8, 
        9, 10, 11, 12, 13, 14, 15, 16, 
        17, 18, 19, 20, 21, 22, 23, 24, 
        25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40
    }; 
    
    // dtype image_in[M*N] = {0, 2, 1 ,1 ,1 , 2, 3 ,3, 0, 1, 0, 1, 1, 2, 1, 4};

    /*dtype image_in[M*N] = 
    {
        1, 4, 0, 1, 3, 1,
        2, 2, 4, 2, 2, 3, 
        1, 0, 1, 0, 1, 0,
        1, 2, 1, 0, 2, 2, 
        2, 5, 3, 1, 2, 5,
        1, 1, 4, 2, 3, 0
    };*/

    dtype image_out[M*N];

    median_filter_sw(image_in, image_out);

    /*for (int i = 0; i < M * N; i++) {
		cout << image_out[i] << " ";
	}*/

    cout << "hw: " << endl;

    hls::stream <dtype> image_in_stream;
    hls::stream <dtype> image_out_stream;

    for (int i = 0; i < M*N; i++)
    {
        image_in_stream.write(image_in[i]);
    }

    median_filter(image_in_stream, image_out_stream);

}

/*

void median_filter_sw(dtype image[M][N], dtype *image_out)
{
    const int new_M = M - F + 1;
    const int new_N = N - F + 1;
    dtype window[F * F];

    // Iterate over the valid region where the filter window fits entirely.
    for (int i = 0; i < new_M; i++)
    {
        for (int j = 0; j < new_N; j++)
        {
            int count = 0;
            for (int k = 0; k < F; k++)
            {
                for (int l = 0; l < F; l++)
                {
                    // Populate the window with valid values from the image.
                    window[count++] = image[i + k][j + l];
                }
            }

            // Compute the median and store the result.
            image_out[i * new_N + j] = median_sw(window, F * F);
        }
    }
}
*/
