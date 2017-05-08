all: edge.h server_and.cpp edge.cpp client.cpp
	g++ -o edge edge.cpp
	g++ -o server_and server_and.cpp
	g++ -o client client.cpp
	g++ -o server_or server_or.cpp
server_or: FORCE
	./server_or
server_and: FORCE
	./server_and
edge: FORCE
	./edge
FORCE: ;
