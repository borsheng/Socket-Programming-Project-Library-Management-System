CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS =

EXECS = serverM serverS serverL serverH client

all: $(EXECS)

serverM: serverM.cpp
	$(CXX) $(CXXFLAGS) -o serverM serverM.cpp $(LDFLAGS)

serverS: serverS.cpp
	$(CXX) $(CXXFLAGS) -o serverS serverS.cpp $(LDFLAGS)

serverL: serverL.cpp
	$(CXX) $(CXXFLAGS) -o serverL serverL.cpp $(LDFLAGS)

serverH: serverH.cpp
	$(CXX) $(CXXFLAGS) -o serverH serverH.cpp $(LDFLAGS)

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp $(LDFLAGS)

clean:
	rm -f $(EXECS)
