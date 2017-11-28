#ifndef FOLLOWER_H
#define FOLLOWER_H

#include <string>
#include <vector>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SERVER_TCP_PORT "3908"
#define FOLLOWER1_UDP_PORT "22008"
#define FOLLOWER2_UDP_PORT "22108"
#define FOLLOWER3_UDP_PORT "22208"
#define FOLLOWER4_UDP_PORT "22308"
#define FOLLOWER5_UDP_PORT "22408"
#define SYSCALL_FAILURE -1
#define MAX_DATA_SIZE 256
#define FOLLOWER_1_HEADER "<Follower1>"
#define FOLLOWER_2_HEADER "<Follower2>"
#define FOLLOWER_3_HEADER "<Follower3>"
#define FOLLOWER_4_HEADER "<Follower4>"
#define FOLLOWER_5_HEADER "<Follower5>"
#define TWEET_A_END_TRANSMISSION "END: <TweetA>"
#define TWEET_B_END_TRANSMISSION "END: <TweetB>"
#define TWEET_C_END_TRANSMISSION "END: <TweetC>"
#define END_TRANSMISSION_MESSAGE "END TWEET TRANSMISSION"


class Follower
{
  public:
  	/* Constructor */
  	Follower ();

  	/* Destructor */
  	~Follower ();

    void Receive();

  	/* function in which all five Followers communicate with the Server */
  	void Connect ();

  private:
  	/* gets the IP address of a socket */
    void *get_in_addr(struct sockaddr *sa);
  
    /* Follower's private data members */
  	struct addrinfo hints;
  	struct addrinfo *serv_info;
  	int sock_fd;
    struct sockaddr_in s;
    std::string follower_header;  
    std::vector<std::string> following;
    std::vector<std::string> likes;
};

#endif
