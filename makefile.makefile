# Variables
CXX = g++
CXXFLAGS = -Wall -std=c++11

# Targets
all: server client

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp

clean:
	rm -f server client

.PHONY: all clean
