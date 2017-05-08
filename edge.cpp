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
#include "edge.h"

// "merge" function I made would be used in line 151,
//  it is used to merge the messages sent back by the two back-end servers
string merge(const string &andr, const string &orr, const string &mask, const string &data){
  string res;
  int p = 0, q=0, r= 0;
  cout << "The edge server start receiving the computation results from Backend-Server OR and Backend-Server AND using UDP over port " << edge_udp_port  << '.'<< endl;
  cout << "The computation results are:" << endl;
  for(int i = 0; i < mask.size();i++){
    int n;
    string tmp;
    if(mask[i]){
      n = orr.find('\n', q);
      tmp = orr.substr(q, n-q +1);
      q = n+1;
    }
    else {
      n = andr.find('\n',p);
      tmp = andr.substr(p, n-p+1);
      p = n+1;
    }
    res += tmp;
    string op;
    if (data[r] == 'a'){
      r = r+4;
      op = " and ";
    }
    else {
      r = r+3;
      op = " or ";
    }
    n = data.find(',',r);
    cout << data.substr(r, n-r);
    cout << op;
    r = n+1;
    n = data.find('\n', r);
    cout << data.substr(r, n-r);
    cout << " = ";
    cout << tmp;
    r = n+1;
  }

  cout << "The edge server has successfully finished receiving all computation results from Backend-Server OR and Backend-Server AND." << endl;
  return res;
}
int main()
{
  // code from Beej's begin
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo;
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;
  // fill in my IP for me
  if ((status = getaddrinfo(NULL, edge_tcp_port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }
  int socket_fd;  
  servinfo = find_server(servinfo, socket_fd);
  int yes = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  listen(socket_fd, 5);
  // code from Beej's end


  int udp_fd, and_fd, or_fd;
  addrinfo *and_server, *or_server, *udp_server;    
  hints.ai_socktype = SOCK_DGRAM; // UDP stream sockets
  getaddrinfo(NULL, edge_udp_port, &hints, &udp_server);
  udp_server = find_server(udp_server, udp_fd);
  bind(udp_fd, udp_server->ai_addr, udp_server->ai_addrlen); // bind udp socket
  
  hints.ai_flags = 0;

  getaddrinfo("127.0.0.1", server_and_port, &hints, &and_server);
  and_server = find_client(and_server, false, and_fd);

  getaddrinfo("127.0.0.1", server_or_port, &hints, &or_server);
  or_server = find_client(or_server, false, or_fd);

  cout << "The edge server is up and running." << endl;

  // The infinite loop begins here
  for(;;){
    char * data;
    // int data_len; int->uint32_t
    uint32_t data_len;
    sockaddr_in their_addr;
    socklen_t addr_size = sizeof their_addr;
    int new_fd = accept(socket_fd, (sockaddr*)&their_addr, &addr_size);
	  //perror("accept");
    recv(new_fd, &data_len, 4, 0);
    data_len = ntohl(data_len);
    data = new char[data_len];
    recv(new_fd,data, data_len, 0);
    int p = 0;
    string and_data, or_data, mask;
    int and_count=0, or_count=0;
    while(p < data_len){
      if (data[p] == 'a'){
	p = p + 4;
	while(p < data_len && data[p] != '\n'){
	  and_data.push_back(data[p]);
	  p++;
	}
	and_data.push_back('\n');
	mask.push_back(0);
	and_count++;
      }
      else if (data[p] == 'o'){
	p = p+3;
	while(p < data_len && data[p] != '\n'){
	  or_data.push_back(data[p]);
	  p++;
	}
	or_data.push_back('\n');
	mask.push_back(1);
	or_count++;
      }
      p++;
    }
    cout << "The edge server has received "<< mask.size() <<  " lines from the client using TCP over port " << edge_tcp_port << '.' << endl;

    // send_batch, recv_batch, send_all are in edge.h
    // send to and receive from or_server
    send_batch(or_data, or_fd, or_server);
    cout << "The edge has successfully sent "<< or_count <<" lines to Backend-Server OR." << endl;  
    string or_res = recv_batch(udp_fd);

    // send to and receive from and_server
    send_batch(and_data,  and_fd, and_server);
    cout << "The edge has successfully sent "<< and_count <<" lines to Backend-Server AND." << endl;
    string  and_res = recv_batch(udp_fd);
    
    // merge the information and send back to the client
    string res = merge(and_res, or_res, mask, data);
    int res_len = htonl(res.size());
    send(new_fd, &res_len, 4,0);
    send_all(res, new_fd);
    cout << "The edge server has successfully finished sending all computation results to the client." << endl;
  }
}
