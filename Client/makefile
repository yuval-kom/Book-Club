CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude -pthread
LDFLAGS:=-lboost_system -lpthread

all: Client
	g++ -o bin/StompBookClubClient.o bin/connectionHandler.o bin/echoClient.o bin/readFromServer.o bin/User.o $(LDFLAGS) 

Client: bin/connectionHandler.o bin/echoClient.o bin/readFromServer.o bin/User.o  
	
bin/connectionHandler.o: src/connectionHandler.cpp
	g++ $(CFLAGS) -o bin/connectionHandler.o src/connectionHandler.cpp

bin/echoClient.o: src/echoClient.cpp
	g++ $(CFLAGS) -o bin/echoClient.o src/echoClient.cpp

bin/readFromServer.o: src/readFromServer.cpp
	g++ $(CFLAGS) -o bin/readFromServer.o src/readFromServer.cpp

bin/User.o: src/User.cpp
	g++ $(CFLAGS) -o bin/User.o src/User.cpp
	
.PHONY: clean
clean:
	rm -f bin/*
