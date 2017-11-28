#include "Server.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

void write_to_file (string tweet);
vector<string> get_tweets_from_file ();
void write_feedback_to_file (string feedback);
vector<string> get_feedback_from_file ();

/* Server Constructor */
Server::Server ()
{
  init ();
}

/* Server Destructor */
Server::~Server ()
{
  /* do I need to call freeaddrinfo for hints as well? */
  freeaddrinfo (serv_info);
}

/* Sets up Server to listen on port 3908 */
void Server::init()
{
  int status; 
  char ipstr[INET6_ADDRSTRLEN];

  /* cite Beej's socket programming guide pages 15/16 */
  memset (&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; 

  /* cite Beej's socket programming guide pages 15/16 */
  if ((status = getaddrinfo("nunki.usc.edu", SERVER_TCP_PORT, &hints, 
                            &serv_info)) != 0) 
  {
    fprintf (stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit (EXIT_FAILURE);
  }

  /* get the socket file descriptor for the server TCP port */
  sock_fd = socket(serv_info->ai_family, SOCK_STREAM, 0);
  if (sock_fd == SYSCALL_FAILURE)
  {
    perror ("socket");
    exit (EXIT_FAILURE);
  }

  /* bind the socket to port 3908 */
  status = bind(sock_fd, serv_info->ai_addr, serv_info->ai_addrlen);
  if (status == SYSCALL_FAILURE)
  {
  	perror ("bind");
    exit (EXIT_FAILURE);
  }

  /* convert IP address of the server socket */
  inet_ntop (serv_info->ai_family, 
             get_in_addr((struct sockaddr *)serv_info->ai_addr), ipstr, 
             sizeof(ipstr));

  sockaddr_in *s = (sockaddr_in *)serv_info->ai_addr;

  cout << "The server has TCP port " << s->sin_port;
  cout << " and IP address " << ipstr << endl;

  /* server socket listens for incoming connections */
  status = listen (sock_fd, BACKLOG);
  if (status == SYSCALL_FAILURE)
  {
    perror ("listen");
    exit (EXIT_FAILURE);
  }
}

/* Communicates with Tweet people and Followers */
bool Server::run()
{
  int tcp_status1, tcp_status2, tcp_status3;
  pid_t pid;
  char buf[MAX_DATA_SIZE];

  addr_size = sizeof (their_addr);

  for (int i=0; i < 3; i++)
  {
    /* accept an incoming connection and give it a new socket descriptor so 
       child socket can communicate with client */
    int new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == SYSCALL_FAILURE)
    {
      perror ("accept");
      exit (EXIT_FAILURE);
    }

    if ((pid = fork()) == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

    /* if pid = 0, executing in the child process */
    if (pid == 0)
    {
      /* don't need the parent (listening) socket to do communication with client */
      close (sock_fd);

      /* recv reads in num_bytes sent over the network by the client */
      int num_bytes = recv(new_fd, buf, MAX_DATA_SIZE-1, 0); 
      if (num_bytes == -1)
      {
        perror ("recv");
        exit (EXIT_FAILURE);
      }   

      /* recv does not null terminate the buffer, so need to null terminate the
         buffer before printing to shell / manipulating buf as a string */
      buf[num_bytes] = '\0';
      //cout << buf << endl;

      string packet(buf);
      istringstream iss(packet);
      bzero (buf, MAX_DATA_SIZE);
      close (new_fd);

      string header;
      string tweet;

      /* first line of a packet is the header, specifying which Tweet the following
         tweets belong to */
      iss >> header;
      //cout << "header: " << header << endl;
      if (header == TWEET_A_HEADER) 
      {
      	cur_header = A;
      	//write_to_file (TWEET_A_HEADER);
      }
      else if (header == TWEET_B_HEADER)
      {
      	cur_header = B;
      	//write_to_file (TWEET_B_HEADER);
      } 
      else if (header == TWEET_C_HEADER) 
      {
        cur_header = C;
        //write_to_file (TWEET_C_HEADER);
      }
      else 
      {
        perror ("header error");
        exit (EXIT_FAILURE);
      }

      /* store all the tweets sent by the client */
      while (getline(iss, tweet))
      {
        //cout << "tweet: " << tweet << endl;
        if (tweet.empty()) continue;

        switch (cur_header)
        {
          case A:
            tweets_a.push_back (tweet);
            write_to_file (tweet);
            break;

          case B:
            tweets_b.push_back (tweet);
            write_to_file (tweet);
            break;

          case C:
            tweets_c.push_back (tweet);
            write_to_file (tweet);
            break;

          default:
            perror ("header error");
            exit (EXIT_FAILURE);
            break;
        }
      }

      /* print to terminal once all tweets have been received for a particular 
         tweet person */
      switch (cur_header)
      {
        case A:
        {
          cout << "Received the tweet list from " << TWEET_A_HEADER << endl;
          string end_message = "END: ";
          end_message += TWEET_A_HEADER;
          write_to_file (end_message);
          break;
        }
        case B:
        {
          cout << "Received the tweet list from " << TWEET_B_HEADER << endl;
          string end_message = "END: ";
          end_message += TWEET_B_HEADER;
          write_to_file (end_message);
          break;
      	}
        case C:
        {
          cout << "Received the tweet list from " << TWEET_C_HEADER << endl;
          string end_message = "END: ";
          end_message += TWEET_C_HEADER;
          write_to_file (end_message);
          break;
        }
        default:
          perror ("header error");
          exit (EXIT_FAILURE);
      }

      exit (EXIT_SUCCESS);
    }

    /* if executing in the parent process, can close the child socket descriptor
       because the parent socket is only responsible for listening for incoming
       connections */
    close (new_fd); 
  }

  /* parent process needs to wait for all child processes to finish execution to 
     ensure all tweets from all clients' TCP connections are received */
  wait (&tcp_status1);
  wait (&tcp_status2);
  wait (&tcp_status3);

  cout << "End of Phase 1 for the server" << endl;

  sleep (10);

  int result;
  char ipstr[INET6_ADDRSTRLEN];
  pid_t pid2;
  int udp_status1, udp_status2, udp_status3, udp_status4, udp_status5;

  memset (&udp_hints, 0, sizeof(udp_hints));
  udp_hints.ai_family = AF_UNSPEC;
  udp_hints.ai_socktype = SOCK_DGRAM;
  udp_hints.ai_flags = AI_PASSIVE; 

  for (int i=0; i < 5; i++) 
  {

   if ((pid2 = fork()) == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

    const char *follower_udp_port;

    if (pid2 == 0)
    {

      switch (i) 
      {
        case 0:
          follower_udp_port = FOLLOWER1_UDP_PORT;
          follower_header = FOLLOWER_1_HEADER;
          break;
        case 1:
          follower_udp_port = FOLLOWER2_UDP_PORT;
          follower_header = FOLLOWER_2_HEADER;
          break;
        case 2:
          follower_udp_port = FOLLOWER3_UDP_PORT;
          follower_header = FOLLOWER_3_HEADER;
          break;
        case 3:
          follower_udp_port = FOLLOWER4_UDP_PORT;
          follower_header = FOLLOWER_4_HEADER;
          break;
        case 4:
          follower_udp_port = FOLLOWER5_UDP_PORT;
          follower_header = FOLLOWER_5_HEADER;
          break;
      }

      /* cite Beej's socket programming guide pages 15/16 */
      if ((result = getaddrinfo("nunki.usc.edu", follower_udp_port, &udp_hints, 
                                &udp_serv_info)) != 0) 
      {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror(result));
        exit (EXIT_FAILURE);
      }

      /* no children sockets required for UDP */
      sock_fd_udp = socket (udp_serv_info->ai_family, udp_serv_info->ai_socktype,
             				udp_serv_info->ai_protocol);

      if (sock_fd_udp == SYSCALL_FAILURE)
      {
        perror ("socket");
        exit (EXIT_FAILURE);
      }

      socklen_t len = sizeof (s_udp);

      vector<string> tweets = get_tweets_from_file ();

      /* send over all tweets in serverTweets.txt from Server to Follower */
      for (int i=0; i < tweets.size(); i++)
      {
        string payload = tweets[i];
        //cout << "payload: " << payload << endl;
        result = sendto (sock_fd_udp, payload.c_str(), strlen(payload.c_str()), 
                         0, udp_serv_info->ai_addr, udp_serv_info->ai_addrlen);

        /* call getsockname after first call to sendto to avoid dynamic UDP port 
           from having value 0 */
        if (i == 0) 
        {
          /* get thes locally bound name of s_udp and store it in s_udp */
          result = getsockname (sock_fd_udp, (struct sockaddr *)&s_udp, 
                               (socklen_t *)&len);

          if (result == SYSCALL_FAILURE)
          {
            perror ("getsockname");
            exit (EXIT_FAILURE);
          }

          /* converts IP address of the Follower socket */
          inet_ntop (udp_serv_info->ai_family, 
                    get_in_addr((struct sockaddr *)udp_serv_info->ai_addr), ipstr, 
                    sizeof(ipstr));

          /* s_udp.sin_port is the dynamically assigned UDP port number of this 
             Server connection */
          cout << "The server has UDP port " << s_udp.sin_port;
          cout << " and IP address " << ipstr << endl; 
        }

        if (payload == TWEET_A_END_TRANSMISSION) 
        {
          cout << "The server has sent " << TWEET_A_HEADER << " to the ";
          cout << follower_header << endl;
        } 
        else if (payload == TWEET_B_END_TRANSMISSION)
        {
          cout << "The server has sent " << TWEET_B_HEADER << " to the ";
          cout << follower_header << endl;
        }
        else if (payload == TWEET_C_END_TRANSMISSION)
        {
          cout << "The server has sent " << TWEET_C_HEADER << " to the ";
          cout << follower_header << endl;
        }
      }

      /* notify follower that it is done receiving tweets from server */
      result = sendto (sock_fd_udp, END_TRANSMISSION_MESSAGE, 
      				   strlen(END_TRANSMISSION_MESSAGE), 0, 
      				   udp_serv_info->ai_addr, udp_serv_info->ai_addrlen);

      if (result == SYSCALL_FAILURE)
      {
      	perror("send");
      	exit (EXIT_FAILURE);
      }

     /* Close the UDP socket connection to the Follwer because no longer needed */
      if ((result = close (sock_fd_udp)) == SYSCALL_FAILURE)
      {
        perror ("close");
        exit (EXIT_FAILURE);
      }

      exit (EXIT_SUCCESS);
    }
  }

  wait (&udp_status1);
  wait (&udp_status2);
  wait (&udp_status3);
  wait (&udp_status4);
  wait (&udp_status5);

  int follower_tcp_status1, follower_tcp_status2, follower_tcp_status3, 
  follower_tcp_status4, follower_tcp_status5;

  /* convert IP address of the server socket */
  inet_ntop (serv_info->ai_family, 
             get_in_addr((struct sockaddr *)serv_info->ai_addr), ipstr, 
             sizeof(ipstr));

  sockaddr_in *s = (sockaddr_in *)serv_info->ai_addr;

  cout << "The server has TCP port " << s->sin_port;
  cout << " and IP address " << ipstr << endl;

  for (int i=0; i < 5; i++)
  {
    /* accept an incoming connection and give it a new socket descriptor so 
       child socket can communicate with client */
    int new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == SYSCALL_FAILURE)
    {
      perror ("accept");
      exit (EXIT_FAILURE);
    }

    if ((pid = fork()) == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

    /* if pid = 0, executing in the child process */
    if (pid == 0)
    {
      /* don't need the parent (listening) socket to do communication with client */
      close (sock_fd);

      int num_bytes = recv(new_fd, buf, MAX_DATA_SIZE-1, 0); 
      if (num_bytes == -1)
      {
        perror ("recv");
        exit (EXIT_FAILURE);
      }   

      buf[num_bytes] = '\0';

      //cout << buf << endl;

      string packet(buf);
      istringstream iss(packet);
      bzero (buf, MAX_DATA_SIZE);
      close (new_fd);

      write_feedback_to_file (packet);

      exit (EXIT_SUCCESS);
    }

    /* if executing in the parent process, can close the child socket descriptor
       because the parent socket is only responsible for listening for incoming
       connections */
    close (new_fd); 
    //int wait_status;
    //wait (&wait_status);
  }

  wait (&follower_tcp_status1);
  wait (&follower_tcp_status2);
  wait (&follower_tcp_status3);
  wait (&follower_tcp_status4);
  wait (&follower_tcp_status5);

  //sleep (10);

  int tweet1_tcp_status_phase2, tweet2_tcp_status_phase2, tweet3_tcp_status_phase2;

  /* accept an incoming connection and give it a new socket descriptor so 
     child socket can communicate with client */
  for (int m=0; m < 3; m++)
  {
    int new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == SYSCALL_FAILURE)
    {
      perror ("accept");
      exit (EXIT_FAILURE);
    }

    //cout << "accepted connection" << endl;
    if ((pid = fork()) == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

    /* if pid = 0, executing in the child process */
    if (pid == 0)
    {
      /* don't need the parent (listening) socket to do communication with client */
      close (sock_fd);

      vector<string> feedback = get_feedback_from_file ();

      for (unsigned int i=0; i < feedback.size(); i++) 
      {
        string payload = feedback[i] + "\n";

        result = send (new_fd, payload.c_str(), strlen(payload.c_str()), 0);
        if (result == SYSCALL_FAILURE)
        {
          perror ("send");
          exit (EXIT_FAILURE);
        }
      }

      result = send (new_fd, END_FEEDBACK_MESSAGE, strlen(END_FEEDBACK_MESSAGE), 0);

      switch (m)
      {
        case 0:
          cout << "The server has sent the feedback result to " << TWEET_A_HEADER;
          cout << endl;
          break;
        case 1:
          cout << "The server has sent the feedback result to " << TWEET_B_HEADER;
          cout << endl;
          break;
        case 2:
          cout << "The server has sent the feedback result to " << TWEET_C_HEADER;
          cout << endl;
          break;
        default:
          cout << "error" << endl;

      }
      /* Close the socket connection to the Server because no longer needed */
      if ((result = close (new_fd)) == SYSCALL_FAILURE)
      {
        perror ("close");
        exit (EXIT_FAILURE);
      }

      exit (EXIT_SUCCESS);
    }

    close (new_fd); 
  }

  wait (&tweet1_tcp_status_phase2);
  wait (&tweet2_tcp_status_phase2);
  wait (&tweet3_tcp_status_phase2);

  cout << "End of Phase 2 for the server" << endl;

  /* remove files created during execution */
  if( remove( "serverTweets.txt" ) != 0 ) {
    perror( "Error deleting file" );
  }
  if( remove( "serverFeedback.txt" ) != 0 ) {
    perror( "Error deleting file" );
  }

  return true;  
}

/* CITE: Beej's programming guide page 28 */
// get sockaddr, IPv4 or IPv6:
void * Server::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int argc, char *argv[])
{
  Server server;

  bool finished = false;

  while (true)
  {
    finished = server.run();
  }

  return 0;
}

void write_to_file (string tweet)
{
  ofstream ofs;

  ofs.open ("serverTweets.txt", ios::app);

  if (!ofs.is_open())
  {
  	cout << "could not open output file serverTweets.txt" << endl;
  	return;
  }

  //cout << "Writing tweet to file..." << endl;

  ofs << tweet << endl;

  ofs.close ();
}

vector<string> get_tweets_from_file ()
{
  ifstream ifs;
  string tweet;
  vector<string> tweets;

  ifs.open ("serverTweets.txt");

  if (!ifs.is_open ()) 
  {
  	cout << "could not open input file serverTweets.txt" << endl;
  	return tweets;
  }

  while (getline (ifs, tweet))
  {
  	tweets.push_back (tweet);
  }

  ifs.close ();

  return tweets;
}

void write_feedback_to_file (string feedback)
{
  ofstream ofs;

  ofs.open ("serverFeedback.txt", ios::app);

  if (!ofs.is_open())
  {
    cout << "could not open output file serverFeedback.txt" << endl;
    return;
  }

  //cout << "Writing tweet to file..." << endl;

  ofs << feedback << endl;

  ofs.close ();
}

vector<string> get_feedback_from_file ()
{
  ifstream ifs;
  string line;
  vector<string> feedback;

  ifs.open ("serverFeedback.txt");

  if (!ifs.is_open ()) 
  {
    cout << "could not open input file serverFeedback.txt" << endl;
    return feedback;
  }

  while (getline (ifs, line))
  {
    if (line.empty())
    {
      continue;
    }
    feedback.push_back (line);
  }

  ifs.close ();

  return feedback;
}
