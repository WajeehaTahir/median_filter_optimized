/*
 * Copyright (C) 2016 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <string.h>

#include "lwip/sockets.h"
#include "netif/xadapter.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "FreeRTOS.h"
#include "task.h"

#include "xparameters.h"
#include "xmedian_filter.h"
#include <xil_cache.h>

typedef int dtype;

#define M (5)
#define N (8)
#define F (3)


#define THREAD_STACKSIZE 1024
/*
 * Max number of telnet connections that this application can serve.
 * The existing implementation does not support closing of an existing telnet.
 * Once a telnet connection is made, it stays for ever.
 */
#define MAX_CONNECTIONS 8
int new_sd[MAX_CONNECTIONS];
int connection_index;

u16_t echo_port = 5001;

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

void print_echo_app_header()
{
    xil_printf("%20s %6d %s\r\n", "echo server",
                        echo_port,
                        "$ telnet <board_ip> 7");

}

/* thread spawned for each connection */
void process_echo_request(void *p)
{
	int sd = *(int *)p;
	/*int RECV_BUF_SIZE = 2048;
	char recv_buf[RECV_BUF_SIZE];
	int n, nwrote;

	while (1) {
		 read a max of RECV_BUF_SIZE bytes from socket
		if ((n = read(sd, recv_buf, RECV_BUF_SIZE)) < 0) {
			xil_printf("%s: error reading from socket %d, closing socket\r\n", __FUNCTION__, sd);
			break;
		}

		 break if the recved message = "quit"
		if (!strncmp(recv_buf, "quit", 4))
			break;

		 break if client closed connection
		if (n <= 0)
			break;

		 handle request
		if ((nwrote = write(sd, recv_buf, n)) < 0) {
			xil_printf("%s: ERROR responding to client echo request. received = %d, written = %d\r\n",
					__FUNCTION__, n, nwrote);
			xil_printf("Closing socket %d\r\n", sd);
			break;
		}
	}

	 close connection
	close(sd);
*/
	//int RECV_BUF_SIZE = 64;
	//	int RECV_BUF_SIZE = 2048;
	//	int RECV_BUF_SIZE = 64 * 1024 * 1024;
	//	int RECV_BUF_SIZE = 128 * 1024 * 1024;
		int RECV_BUF_SIZE = M*N*sizeof(dtype);
	//	int max_transfers = 1;

		int max_transfers = 16;
		int curr_tr;
		int bytes_left;
		int status;
		int *recv_buf_int = memalign(16, RECV_BUF_SIZE);
		int *snd_buf_int = recv_buf_int;
		char *buff;
		int n, nwrote;

		while (1) {
			//start receiving data
			//for (curr_tr = 0; curr_tr < max_transfers; curr_tr++) {
				xil_printf("Iteration: %d receiving\r\n", curr_tr);
				bytes_left = RECV_BUF_SIZE;
				buff = (char *) recv_buf_int;
				status = 0;
				//receive packets until all data arrived
				while (bytes_left > 0) {
					if ((n = read(sd, buff, bytes_left)) < 0) {
						xil_printf(
								"%s: error reading from socket %d, closing socket\r\n",
								__FUNCTION__, sd);
						status = -1;
						break;
					}
					if (n <= 0) {
						xil_printf("Connection closed.\r\n");
						status = -2;
						break;
					}
					bytes_left -= n;
					buff += n;
					//xil_printf("Packet received, length: %d bytes left: %d bytes\r\n",
					//		n, bytes_left);
				}
				if (status < 0) {
					break;
				}
			//}
			/*if (status < 0) {
				break;
			}*/

			xil_printf("Computing\r\n");

			dtype image_out_sw[(M - F + 1) * (N - F + 1)], image_out_hw[(M - F + 1) * (N - F + 1)];

			median_filter_sw(recv_buf_int, image_out_sw);

			median_filter_fpga(recv_buf_int, image_out_hw);

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


			/*for (int i = 0; i < RECV_BUF_SIZE / 4; i++) {
				snd_buf_int[i] = recv_buf_int[i] + 2;
				//xil_printf("recv_buf[%d] %d, snd_buf[%d] %d\r\n", i,
			}*/
			xil_printf("Finished\r\n");

			//star sending back
			//for (curr_tr = 0; curr_tr < max_transfers; curr_tr++) {
				xil_printf("Iteration: %d sending\r\n", curr_tr);
				bytes_left = RECV_BUF_SIZE;
				buff = (char *) image_out_hw;
				status = 0;
				//send packets until no data left
				while (bytes_left > 0) {
					//send packet
					if ((nwrote = write(sd, buff, bytes_left)) < 0) {
						xil_printf(
								"%s: ERROR responding to client echo request. received = %d, written = %d\r\n",
								__FUNCTION__, n, nwrote);
						xil_printf("Closing socket %d\r\n", sd);
						status = -3;
						break;
					}
					bytes_left -= nwrote;
					buff += nwrote;
					//xil_printf("%d bytes sent back left: %d bytes\r\n", nwrote,
					//		bytes_left);
				}
				if (status < 0) {
					break;
				}
			/*}
			if (status < 0) {
				break;
			}*/

		}
		free(recv_buf_int);
		xil_printf("Memory freed.\r\n");
		/* close connection */
		close(sd);

	vTaskDelete(NULL);
}

void echo_application_thread()
{
	int sock;
	int size;
#if LWIP_IPV6==0
	struct sockaddr_in address, remote;

	memset(&address, 0, sizeof(address));

	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return;

	address.sin_family = AF_INET;
	address.sin_port = htons(echo_port);
	address.sin_addr.s_addr = INADDR_ANY;
#else
	struct sockaddr_in6 address, remote;

	memset(&address, 0, sizeof(address));

	address.sin6_len = sizeof(address);
	address.sin6_family = AF_INET6;
	address.sin6_port = htons(echo_port);

	memset(&(address.sin6_addr), 0, sizeof(address.sin6_addr));

	if ((sock = lwip_socket(AF_INET6, SOCK_STREAM, 0)) < 0)
		return;
#endif

	if (lwip_bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
		return;

	lwip_listen(sock, 0);

	size = sizeof(remote);

	while (1) {
		if ((new_sd[connection_index] = lwip_accept(sock, (struct sockaddr *)&remote, (socklen_t *)&size)) > 0) {
			sys_thread_new("echos", process_echo_request,
				(void*)&(new_sd[connection_index]),
				THREAD_STACKSIZE,
				DEFAULT_THREAD_PRIO);
			if (++connection_index>= MAX_CONNECTIONS) {
				break;
			}
		}
	}
	xil_printf("Maximum number of connections reached, No further connections will be accepted\r\n");
	vTaskSuspend(NULL);
}
