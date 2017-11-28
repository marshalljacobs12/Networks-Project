CC = g++
CFLAGS = -lsocket -lnsl -lresolv

all: server tweet follower

server:
	$(CC) $(CFLAGS) -o server.out Server.cpp

tweet:
	$(CC) $(CFLAGS) -o tweet.out Tweet.cpp

follower:
	$(CC) $(CFLAGS) -o follower.out Follower.cpp

.PHONY: clean

clean:
	-@rm *.out
