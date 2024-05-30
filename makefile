CXX = g++
CXXFLAGS = -std=c++11 -pthread

all: servidor cliente

servidor: servidor.cpp
	$(CXX) $(CXXFLAGS) -o servidor servidor.cpp

cliente: cliente.cpp
	$(CXX) $(CXXFLAGS) -o cliente cliente.cpp

clean:
	rm -f servidor cliente
