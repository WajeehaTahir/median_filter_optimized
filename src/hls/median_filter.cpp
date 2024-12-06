#include "median_filter.h"
#include <iostream>

using namespace std;

dtype* sort_ascending(dtype * a, int n)
{
    sort_loop_1:
    for(int i=0; i<n; i++)
    {
        sort_loop_2:
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

dtype median(dtype window[F][F], int n)
{
    dtype window_array[F*F];

    // cout << "window: " << endl;

    int count = 0;
    median_loop_1:
    for (int i = 0; i < F; i++)
    {
        median_loop_2:
        for (int j = 0; j < F; j++)
        {
            // cout << window[i][j] << " ";
            window_array[count] = window[i][j];
            count++;

        }
        // cout << endl;
    }

    return sort_ascending(window_array, n)[n/2];
}

void median_filter(hls::stream <dtype> &image_in, hls::stream <dtype> &image_out)
{
#pragma HLS INTERFACE mode=s_axilite port=return
#pragma HLS STREAM variable=image_out
#pragma HLS STREAM variable=image_in
    dtype window[F][F];
    dtype buffer[2][N];
    dtype temp, temp_col[3];

    filter_loop_1:
    for (int y = 0; y < M; y++)
    {
        filter_loop_2:
        for (int x = 0; x < N; x++)
        {
#pragma HLS PIPELINE
            image_in.read(temp);

            temp_col[0] = buffer[0][x];
            temp_col[1] = buffer[1][x];

            window[0][0] = window[0][1];
            window[0][1] = window[0][2];
            window[0][2] = temp_col[0];

            window[1][0] = window[1][1];
            window[1][1] = window[1][2];

            window[1][2] = temp_col[1];
            buffer[0][x] = temp_col[1];

            window[2][0] = window[2][1];
            window[2][1] = window[2][2];

            window[2][2] = temp;
            buffer[1][x] = temp;

            if (x > 1 && y > 1)  
            {
                dtype median_value = median(window, F*F);

                // cout << "median: " << median_value << endl;
                image_out.write(median_value);
            }
        }
    }
}
