// Name: Chaoran Lu
// StudentID: 6524-2400-52
// Code for "and" server

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
#include "edge.h"

// function clac_and is used in function calc in order to do the "and" operation
string clac_and(string q){
  int p = q.find(',');
  string res = q.substr(0, p);
  for(int i = q.size()-1, j = res.size()-1;j>=0;i--, j--){
    if (i <= p || q[i]=='0' )
      res[j] ='0';
  }
  
  int s = res.find('1');
  if (s==string::npos)
    res = "0";
  else
    res =  res.substr(s, res.size());
  cout << q.substr(0,p) << " and " << q.substr(p+1) << " = " << res << endl;
  return res;
}

// function calc is used in line 90 in order to process the string received and pass it to 
// the above clac_and function and return the final result calculated
string calc(string data){
  int p=0;
  string res;
  int n = 0;
  while(p < data.size()){
    int i = 0;
    while(p+i < data.size() && data[p+i] != '\n'){
      i++;
    }

    res += clac_and(data.substr(p, i)) + '\n';
    n++;
    p = p+i+1;

  }
  cout << "The Server AND has successfully received " << n <<" lines from the edge server and finished all AND computations.\n";
  return res;
}

int main()
{
  // code from Beej's begin
  int status;
  struct addrinfo hints;
  struct addrinfo *servinfo, *edge_server;
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE;
  // fill in my IP for me
  if ((status = getaddrinfo(NULL, server_and_port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }
  int socket_fd, edge_fd;
  servinfo = find_server(servinfo, socket_fd);
  bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);
  // code from Beej's end
  
  hints.ai_flags = 0;
  getaddrinfo("127.0.0.1", edge_udp_port, &hints, &edge_server);
  find_client(edge_server, false, edge_fd);

  cout << "The Server AND is up and running using UDP on port " << server_and_port << ".\n";

  // receive the requests in an infinite loop
  for(;;){
    string data;
    data = recv_batch(socket_fd);
    cout << "The Server AND start receiving lines from the edge server for AND computation. The computation results are:\n";
    string res = calc(data);
    send_batch(res, edge_fd, edge_server);
    cout << "The Server AND has successfully finished sending all computation results to the edge server.\n";
  }
  
}
