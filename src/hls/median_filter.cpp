#include "median_filter.h"

dtype* sort_ascending(dtype * a, int n)
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

dtype median(dtype* window, int n)
{
    return sort_ascending(window, n)[n/2];
}

void median_filter(dtype *image_in, dtype *image_out)
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
            for (int k = -F/2; k < F/2; k++)
            {
                for (int l = -F/2; l < F/2; l++)
                {
                    if ((i+k) < 0 || (j+k) < 0 || (i+k) > M || (j+k) > N)
                    {
                        window[count] = 0;
                    }
                    else {
                        window[count] = image[i+k][j+l];
                    }
                    
                    count++;
                }
            }

            filtered_image[i][j] = median(window, F*F);
        }
    }

    save_output: for (int i = 0; i < M * N; i++) {
		image_out[i] = filtered_image[i / N][i % N];
	}

}
