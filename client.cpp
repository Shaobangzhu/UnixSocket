// Name: Chaoran Lu
// StudentID: 6524-2400-52
// Code for Client

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
using namespace std;


int main(int argc, char *argv[])
{
  // read the input file
  freopen(argv[1], "r", stdin);
  vector<string> jobs;
  string s;
  while(getline(cin, s)){
    if (s.size() > 0){
      jobs.push_back(s);
    }
  }
  // code from Beej's begin
  // find local IP and port number
  int status;
  int socket_fd;
  addrinfo hints;
  addrinfo *servinfo;
  memset(&hints, 0, sizeof hints); // make sure the struct is empty
  hints.ai_family = AF_INET; // don't care IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; //TCP stream socket
  status = getaddrinfo("127.0.0.1", edge_tcp_port, &hints, &servinfo);
  servinfo = find_client(servinfo, true, socket_fd);
  // code from Beej's ends;
  
  string stream;
  for (int i = 0; i < jobs.size(); i++){
    stream += jobs[i] +'\n';
  }
  int data_len = htonl(stream.size());

  // boot up message
  cout << "The client is up and running." << endl;
  // send numbers to edge server
  send(socket_fd, &data_len, 4, 0);
  send_all(stream, socket_fd);
  cout << "The client has successfully finished sending " << jobs.size() << " lines to the edge server."<< endl;  
  
  // receive numbers from the edge server
  uint32_t res_len = 0;
  recv(socket_fd, &res_len, 4,0 );
  res_len = ntohl(res_len);
  char *res = new char[res_len+2];
  res[res_len] =0;
  recv(socket_fd, res, res_len, 0);
  cout << "The client has successfully finished receiving all computation results from the edge server." << endl;
  cout << "The final computation result are:" << endl;
  printf("%s", res);
}
