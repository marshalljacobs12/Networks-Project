#ifndef TWEET_H
#define TWEET_H

#include <string>
#include <vector>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SERVER_TCP_PORT "3908"
#define SYSCALL_FAILURE -1
#define TWEET_A_HEADER "<TweetA>"
#define TWEET_B_HEADER "<TweetB>"
#define TWEET_C_HEADER "<TweetC>"
#define MAX_DATA_SIZE 1024
#define END_FEEDBACK_MESSAGE "END_FEEDBACK_MESSAGE"
#define FOLLOWER_1_HEADER "<Follower1>"
#define FOLLOWER_2_HEADER "<Follower2>"
#define FOLLOWER_3_HEADER "<Follower3>"
#define FOLLOWER_4_HEADER "<Follower4>"
#define FOLLOWER_5_HEADER "<Follower5>"

class Tweet
{
  public:
  	/* Constructor */
  	Tweet ();

  	/*Destructor */
  	~Tweet ();

    /* function in which all three Tweet people communicate with the Server */
    void Connect ();

  private:
    /* gets the IP address of a socket */
    void *get_in_addr(struct sockaddr *sa);
    
    /* Tweet private data members */
  	struct addrinfo hints;
  	struct addrinfo *serv_info;
  	int sock_fd;
    std::vector<std::string> tweets;
    std::string tweet_header;
    struct sockaddr_in s;
};

#endif
