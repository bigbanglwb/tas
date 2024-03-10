/*
 * Copyright 2019 University of Washington, Max Planck Institute for
 * Software Systems, and The University of Texas at Austin
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <tas_sockets.h>

const int print_timer_period = 1;
#define MAX_BYTES 2048
int main(int argc, char *argv[])
{
  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  ssize_t ret;
  double prev_tsc,start_tsc,start_cap_tsc;
  unsigned int  rx_pkts = 0;
  char buf1[MAX_BYTES];
  if (tas_init() != 0) {
    perror("tas_init failed");
    return -1;
  }

  if ((listenfd = tas_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    abort();
  }

  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(1234);

  if (tas_bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("bind failed");
    abort();
  }

  if (tas_listen(listenfd, 10)) {
    perror("listen failed");
    abort();
  }

  if ((connfd = tas_accept(listenfd, NULL, NULL)) < 0) {
    perror("accept failed");
    abort();
  }
  start_tsc = (double)(clock()) / CLOCKS_PER_SEC;
  prev_tsc = 0.0;
  start_cap_tsc = 0.0;
  while(1) {
    double curr_tsc = (double)(clock()) / CLOCKS_PER_SEC;
    double diff_tsc = (curr_tsc - prev_tsc);
    // read packet from buffer
    ret = tas_read(connfd, buf1, sizeof(buf1));
    
    // after 3 sceond, start capture
    if (curr_tsc - start_tsc < 3) 
      continue;
    else if (start_cap_tsc == 0)
      start_cap_tsc = curr_tsc;
    
    // pkt accumulated
    if (ret > 0)
      rx_pkts += 1;

    // print stats every 1 second
    if (diff_tsc >= print_timer_period)
    {   
        double spend_time = curr_tsc - start_cap_tsc; 
        double send_pps = rx_pkts  / spend_time;
        
        printf("recv %.2f Kpps\n",send_pps/1000);
        prev_tsc = curr_tsc;
    }
  }
}
