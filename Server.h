#ifndef SERVER_H
#define SERVER_H

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
#define BACKLOG 10
#define MAX_DATA_SIZE 256
#define TWEET_A_HEADER "<TweetA>"
#define TWEET_B_HEADER "<TweetB>"
#define TWEET_C_HEADER "<TweetC>"
#define FOLLOWER_1_HEADER "<Follower1>"
#define FOLLOWER_2_HEADER "<Follower2>"
#define FOLLOWER_3_HEADER "<Follower3>"
#define FOLLOWER_4_HEADER "<Follower4>"
#define FOLLOWER_5_HEADER "<Follower5>"
#define TWEET_A_END_TRANSMISSION "END: <TweetA>"
#define TWEET_B_END_TRANSMISSION "END: <TweetB>"
#define TWEET_C_END_TRANSMISSION "END: <TweetC>"
#define END_TRANSMISSION_MESSAGE "END TWEET TRANSMISSION"
#define END_FEEDBACK_MESSAGE "END_FEEDBACK_MESSAGE"

enum header
{
  A,
  B,
  C,
};

class Server
{
  public:
  	/* Constructor */
  	Server ();

  	/*Destructor */
  	~Server ();

    /* function in which server does all its work */
  	bool run ();

  private:
    /* sets up the server */
  	void init ();
    /* gets the IP address of a socket */
    void *get_in_addr(struct sockaddr *sa);

    /* Server private data members */
  	struct addrinfo hints, udp_hints;
  	struct addrinfo *serv_info, *udp_serv_info;
  	int sock_fd, sock_fd_udp;
    struct sockaddr_in s_udp;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    std::vector<std::string> tweets_a, tweets_b, tweets_c;
    enum header cur_header;
    std::string follower_header;
};

#endif
