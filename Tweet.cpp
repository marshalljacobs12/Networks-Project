#include "Tweet.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

bool parse (string file_name, vector<string>& tweets);
void printFeedback (string header, char* feedback);

/* Tweet Constructor */
Tweet::Tweet ()
{
  int status; 

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
}

/* Tweet Destructor */
Tweet::~Tweet ()
{
  /* do I need to call freeaddrinfo for hints as well? */
  freeaddrinfo (serv_info); 
}

/* All three Tweet people communicate with the server via forking child 
   processes */
void Tweet::Connect ()
{
  pid_t pid;
  char ipstr[INET6_ADDRSTRLEN];
  int len;
  int result;
  int tcp_status1, tcp_status2, tcp_status3;

  /* need to call fork three times for three Tweet people */
  for (int i=0; i < 3; i++)
  {
    if ((pid = fork()) == SYSCALL_FAILURE)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

    /* in the child process */
    if (pid == 0)
    {
      /* The process for Tweet person A's communication is created during the
         first fork call, Tweet person B's process in the second fork call and
         Tweet person C's process in the third fork call */
      switch (i)
      {
        case 0:
          if (!parse("TweetA.txt", tweets))
            exit (EXIT_FAILURE);
          tweet_header = TWEET_A_HEADER;
          break;
        case 1:
          if (!parse("TweetB.txt", tweets))
            exit (EXIT_FAILURE);
          tweet_header = TWEET_B_HEADER;
          break;
        case 2:
          if (!parse("TweetC.txt", tweets))
            exit (EXIT_FAILURE);
          tweet_header = TWEET_C_HEADER;
          break;
        /* don't really need the default case */
        default:
          cout << "Something went wrong" << endl;
          break;
      }

      /* get the socket file descriptor for the Server TCP port */
      sock_fd = socket (serv_info->ai_family, serv_info->ai_socktype, 
                        serv_info->ai_protocol);

      if (sock_fd == SYSCALL_FAILURE)
      {
        perror ("socket");
        exit (EXIT_FAILURE);
      }

      /* each Tweet person should connect with the Server over this TCP port */
      result = connect (sock_fd, serv_info->ai_addr, serv_info->ai_addrlen);

      if (result == SYSCALL_FAILURE)
      {
        perror ("connect");
        exit (EXIT_FAILURE);
      }

      len = sizeof(s);

      /* get thes locally bound name of s and store it in s*/
      result = getsockname (sock_fd, (struct sockaddr *)&s, (socklen_t *)&len);

      if (result == SYSCALL_FAILURE)
      {
        perror ("getsockname");
        exit (EXIT_FAILURE);
      }

      /* converts IP address of the server socket */
      inet_ntop (serv_info->ai_family, 
                 get_in_addr((struct sockaddr *)serv_info->ai_addr), ipstr, 
                 sizeof(ipstr));

      /* s.sin_port is the dynamically assigned TCP port number of this Tweet
         person */
      cout << tweet_header << " has TCP port " << s.sin_port;
      cout << " and IP address " << ipstr << endl;

      cout << tweet_header << " is now connected to the server" << endl;

      /* send the header before sending the tweets so the Server knows how to 
         store them */
      string header = tweet_header + "\n";
      result = send (sock_fd, header.c_str(), strlen(header.c_str()), 0);
      if (result == SYSCALL_FAILURE)
      {
        perror ("send");
        exit (EXIT_FAILURE);
      }

      /* send over all this Tweet person's tweets from its input file */
      for (int j=0; j < tweets.size(); j++)
      {
        string payload = tweets[j] + "\n";

        result = send (sock_fd, payload.c_str(), strlen(payload.c_str()), 0);
        if (result == SYSCALL_FAILURE)
        {
          perror ("send");
          exit (EXIT_FAILURE);
        }
        cout << tweet_header << " has sent <" << tweets[j] << "> to the server";
        cout << endl;
      }

      /* This Tweet person has sent all its tweets to the server */
      cout << "Updating the server is done for " << tweet_header << endl;

      cout << "End of Phase 1 for " << tweet_header << endl;
      
      /* Close the socket connection to the Server because no longer needed */
      if ((result = close (sock_fd)) == SYSCALL_FAILURE)
      {
        perror ("close");
        exit (EXIT_FAILURE);
      }

      exit (EXIT_SUCCESS);
    }
    int wait_status;
    wait (&wait_status);
  }

  /* wait for all three child processes to finish initial TCP communications with
     Server before parent process exits */
  wait (&tcp_status1);
  wait (&tcp_status2);
  wait (&tcp_status3);

  /* PHASE 2 */
  int tcp_status_phase2_1, tcp_status_phase2_2, tcp_status_phase2_3;
  char buf[MAX_DATA_SIZE];

  sleep (15);

  for (int i=0; i < 3; i++)
  {
    if ((pid = fork()) == SYSCALL_FAILURE)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

    if (pid == 0)
    {
      /* The process for Tweet person A's communication is created during the
         first fork call, Tweet person B's process in the second fork call and
         Tweet person C's process in the third fork call */
      switch (i)
      {
        case 0:
          tweet_header = TWEET_A_HEADER;
          break;
        case 1:
          tweet_header = TWEET_B_HEADER;
          break;
        case 2:
          tweet_header = TWEET_C_HEADER;
          break;
        /* don't really need the default case */
        default:
          cout << "Something went wrong" << endl;
          break;
      }
      /* get the socket file descriptor for the Server TCP port */
      sock_fd = socket (serv_info->ai_family, serv_info->ai_socktype, 
                        serv_info->ai_protocol);

      if (sock_fd == SYSCALL_FAILURE)
      {
        perror ("socket");
        exit (EXIT_FAILURE);
      }

      /* each Tweet person should connect with the Server over this TCP port */
      result = connect (sock_fd, serv_info->ai_addr, serv_info->ai_addrlen);

      sleep(1);

      if (result == SYSCALL_FAILURE)
      {
        perror ("connect");
        exit (EXIT_FAILURE);
      }

      len = sizeof(s);

      /* get thes locally bound name of s and store it in s*/
      result = getsockname (sock_fd, (struct sockaddr *)&s, (socklen_t *)&len);

      if (result == SYSCALL_FAILURE)
      {
        perror ("getsockname");
        exit (EXIT_FAILURE);
      }

      /* converts IP address of the server socket */
      inet_ntop (serv_info->ai_family, 
                 get_in_addr((struct sockaddr *)serv_info->ai_addr), ipstr, 
                 sizeof(ipstr));

      /* s.sin_port is the dynamically assigned TCP port number of this Tweet
         person */
      cout << tweet_header << " has TCP port " << s.sin_port;
      cout << " and IP address " << ipstr <<  " for Phase 2" << endl;

      /* recv reads in num_bytes sent over the network by the client */
      int num_bytes;
      //while(true)
      //{
        num_bytes = recv(sock_fd, buf, MAX_DATA_SIZE-1, 0); 
        if (num_bytes == -1)
        {
          perror ("recv");
          exit (EXIT_FAILURE);
        }   

        /* recv does not null terminate the buffer, so need to null terminate the
           buffer before printing to shell / manipulating buf as a string */
        buf[num_bytes] = '\0';
        //cout << "buf: " << buf << endl;
        //if (strcmp(buf, END_FEEDBACK_MESSAGE) == 0) break;
      //}

      printFeedback (tweet_header, buf);

      /* Close the socket connection to the Server because no longer needed */
      if ((result = close (sock_fd)) == SYSCALL_FAILURE)
      {
        perror ("close");
        exit (EXIT_FAILURE);
      }

      exit (EXIT_SUCCESS);
    }
    int wait_status, wait_status2;
    wait (&wait_status);
  }

  wait (&tcp_status_phase2_1);
  wait (&tcp_status_phase2_2);
  wait (&tcp_status_phase2_3);
}

/* Prints the feedback for the tweet specified by header */
void printFeedback (string header, char* feedback)
{
  string header_;
  string cur_follower = FOLLOWER_1_HEADER;
  bool should_evaluate_next_token = false;
  string evaluate_criteria;

  if (header.compare(TWEET_A_HEADER) == 0)
  {
    header_ = "TweetA";
  }
  else if (header.compare(TWEET_B_HEADER) == 0)
  {
    header_ = "TweetB";
  }
  else if (header.compare(TWEET_C_HEADER) == 0)
  {
    header_ = "TweetC";
  }
  //cout << "header_: " << header_ << endl;

  char * pch;
  pch = strtok (feedback,":\n");

  while (pch != NULL)
  {
    //printf ("%s\n",pch);
    pch = strtok (NULL, ":\n");
    if (strcmp(pch, FOLLOWER_1_HEADER) == 0) {
      cur_follower = FOLLOWER_1_HEADER;
      should_evaluate_next_token = false;
    }
    else if (strcmp(pch, FOLLOWER_2_HEADER) == 0) {
      cur_follower = FOLLOWER_2_HEADER;
      should_evaluate_next_token = false;
    }
    else if (strcmp(pch, FOLLOWER_3_HEADER) == 0) {
      cur_follower = FOLLOWER_3_HEADER;
      should_evaluate_next_token = false;
    }
    else if (strcmp(pch, FOLLOWER_4_HEADER) == 0) {
      cur_follower = FOLLOWER_4_HEADER;
      should_evaluate_next_token = false;
    }
    else if (strcmp(pch, FOLLOWER_5_HEADER) == 0) {
      cur_follower = FOLLOWER_5_HEADER;
      should_evaluate_next_token = false;
    }
    else if (strcmp(pch, "FOLLOW") == 0) {
      should_evaluate_next_token = true;
      evaluate_criteria = "FOLLOW";
      continue;
    }
    else if (strcmp(pch, "LIKE") == 0) {
      should_evaluate_next_token = true;
      evaluate_criteria = "LIKE";
      continue;
    }
    else
    {
      if (should_evaluate_next_token) {
        if (evaluate_criteria.compare("FOLLOW")) {
          if (strcmp(pch, header_.c_str()) == 0) {
            cout << cur_follower << " has liked " << header << endl;
          }
        } else if (evaluate_criteria.compare("LIKE")) {
          if (strcmp(pch, header_.c_str()) == 0) {
            cout << cur_follower << " is following " << header << endl;
          }
        }
      }
    }
  }
}

/* CITE: Beej's programming guide page 28 
   get sockaddr, IPv4 or IPv6: */
void * Tweet::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int argc, char *argv[])
{
  Tweet tweet;

  tweet.Connect ();

  return 0;
}

/* parses a Tweet person's input file with their tweets and stores the tweets in
   the vector tweets. Returns false if input file could not be parsed */
bool parse (string file_name, vector<string>& tweets)
{
  ifstream ifs;
  string line;

  ifs.open (file_name.c_str());

  if (!ifs.is_open())
  {
    cout << "could not open file: " << file_name << endl;
    return false;
  }   

  while (getline(ifs, line))
  {
    if (line.empty())
    {
      continue;
    }
    tweets.push_back (line);
  }

  ifs.close ();
  return true;
}

