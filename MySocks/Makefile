.PHONY:all
all:server client

server:server.cpp common.cpp common.h transfer.h
	g++ -o $@ $^ -lpthread

client:common.cpp common.h protocol.h client.cpp transfer.h
	g++ -o $@ $^  -lpthread



.PHONY:clean
clean:
	rm -f server client
