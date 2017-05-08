// Name: Chaoran Lu
// StudentID: 6524-2400-52
// Code for edge server

#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <algorithm>
using namespace std;

const char* edge_tcp_port="23052";
const char* edge_udp_port="24052";
const char* server_and_port="22052";
const char* server_or_port="21052";

const  int packet_size = 500;


//code from beej's
addrinfo* find_server(addrinfo * servinfo, int &sockfd){
  addrinfo *p;
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
      continue;
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    return p;
 }
}

addrinfo * find_client(addrinfo * servinfo, bool tcp, int &sockfd){
  addrinfo *p;
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }
    if (tcp && connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      continue;
    }
    return p;
  }
}

void send_all(const string &stream, int fd){
  int total = 0;
  const char *data = stream.c_str();
  while(total < stream.size()){
    int success_byte = send(fd, data+total, stream.size() - total, 0 );
    total += success_byte;
  }
}

//code from beej's end

// send the message from edge to the back_end server
void send_batch(const string & data, int fd, addrinfo *addr){
  char buf[512];
  const char *cdata= data.c_str();
  int n = data.size();
  int packet_count = (n+packet_size-1)/packet_size;
  *(uint32_t*)buf = htonl(packet_count);
  for (int i = 0; i < packet_count; i++){
    *(uint32_t*)(buf+4) = htonl(i);
    memset(buf+12, 0, packet_size);
    memcpy(buf+12, cdata+ i*packet_size, min(packet_size, n - i*packet_size));
    sendto(fd, buf, sizeof buf, 0, addr->ai_addr, addr->ai_addrlen);
  }
}

// receive the message the back_end server
string recv_batch(int fd){
  char *data;
  int total = -1;
  int recv_count = 0;
  for(;;){
    sockaddr their_addr;
    socklen_t addr_size;
    char buf[512];
    recvfrom(fd, buf, sizeof buf, 0, &their_addr, &addr_size);
    recv_count++;
    if (total  <0){
      total = ntohl(*(uint32_t*)buf);
      data = new char[total * packet_size];
    }
    int index = ntohl(*(uint32_t*)(buf + 4));
    memcpy(data + packet_size * index, buf + 12, packet_size);
    if (recv_count >= total)
      break;
  }
  string r;
  for (int i = 0; i < total * packet_size; i++){
    if(data[i] != 0)
      r.push_back(data[i]);
  }
  return r;
}
