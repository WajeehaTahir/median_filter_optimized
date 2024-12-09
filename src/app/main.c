#include <stdio.h>
#include "xparameters.h"
#include "xil_printf.h"
#include "xmedian_filter.h"
#include <xil_cache.h>

typedef int dtype;

#define M (5)
#define N (8)
#define F (3)

void median_filter_fpga(dtype *image_in, dtype *image_out)
{
	XMedian_filter hw;
	XMedian_filter_Initialize(&hw, XPAR_MEDIAN_FILTER_0_S_AXI_CONTROL_BASEADDR);
	XMedian_filter_Set_image_in(&hw, (u64) image_in);
	XMedian_filter_Set_image_out(&hw, (u64) image_out);

	Xil_DCacheFlushRange(image_in, M*N*sizeof(dtype));
	Xil_DCacheInvalidateRange(image_out, (M-F+1)*(N-F+1)*sizeof(dtype));

	XMedian_filter_Start(&hw);

    while(!XMedian_filter_IsDone(&hw));
}

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

void median_filter_sw(dtype image_in[M*N], dtype *image_out)
{
    dtype image[M][N];

    load_image: for (int i = 0; i < M * N; i++) {
		image[i / N][i % N] = image_in[i];
	}

    const int new_M = M - F + 1;
    const int new_N = N - F + 1;
    dtype window[F * F];

    for (int i = 0; i < new_M; i++)
    {
        for (int j = 0; j < new_N; j++)
        {
            int count = 0;
            for (int k = 0; k < F; k++)
            {
                for (int l = 0; l < F; l++)
                {
                    window[count++] = image[i + k][j + l];
                }
            }

            image_out[i * new_N + j] = median_sw(window, F * F);
        }
    }
}

int main()
{
    /*dtype image_in_1[M][N] = {
        {1, 2, 3, 4, 5, 6, 7, 8},
        {9, 10, 11, 12, 13, 14, 15, 16},
        {17, 18, 19, 20, 21, 22, 23, 24},
        {25, 26, 27, 28, 29, 30, 31, 32},
        {33, 34, 35, 36, 37, 38, 39, 40}
    }; */
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

    dtype image_out_sw[(M - F + 1) * (N - F + 1)], image_out_hw[(M - F + 1) * (N - F + 1)];

    median_filter_sw(image_in, image_out_sw);

    median_filter_fpga(image_in, image_out_hw);

    int flag = 1;

    for(int i = 0; i <(M-F+1)*(N-F+1); i++)
    {
        if (image_out_sw[i] != image_out_hw[i])
        {
        	xil_printf("hw and sw implementations do not match \r\n");
        	flag = 0;
        }

        xil_printf("%d %d \r\n", image_out_sw[i], image_out_hw[i]);
    }

    if(flag) xil_printf("hw and sw implementations match");

}
