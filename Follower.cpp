#include "Follower.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

vector<string> parseFollowing (string file_name);
vector<string> parseLikes (string file_name);

Follower::Follower ()
{
  //int status; 
  //char ipstr[INET6_ADDRSTRLEN];
  //struct sockaddr_storage their_storage;
  //socklen_t their_len = sizeof (their_storage);

  /* cite Beej's socket programming guide pages 15/16 */
  //memset (&hints, 0, sizeof(hints));
  //hints.ai_family = AF_UNSPEC;
  //hints.ai_socktype = SOCK_DGRAM;
  //hints.ai_flags = AI_PASSIVE;

  /* cite Beej's socket programming guide pages 15/16 */
  //if ((status = getaddrinfo("nunki.usc.edu", FOLLOWER1_UDP_PORT, &hints, 
  //                          &serv_info)) != 0) 
  //{
    //fprintf (stderr, "getaddrinfo: %s\n", gai_strerror(status));
    //exit (EXIT_FAILURE);
  //}	

  //sock_fd = socket (serv_info->ai_family, serv_info->ai_socktype, 
  //                  serv_info->ai_protocol);

  //if (sock_fd == SYSCALL_FAILURE)
  //{
    //perror ("socket");
    //exit (EXIT_FAILURE);
  //}

  //status = bind (sock_fd, serv_info->ai_addr, serv_info->ai_addrlen);

  /* convert IP address of the server socket */
  //inet_ntop (serv_info->ai_family, 
  //           get_in_addr((struct sockaddr *)serv_info->ai_addr), ipstr, 
  //           sizeof(ipstr));

  //sockaddr_in *s = (sockaddr_in *)serv_info->ai_addr;

  //cout << "<FollowerX> has UDP port " << s->sin_port;
  //cout << " and IP address " << ipstr << endl;
}

Follower::~Follower ()
{
  /* do I need to call freeaddrinfo for hints as well? */
}

void Follower::Receive ()
{
  int status; 
  int udp_status1, udp_status2, udp_status3, udp_status4, udp_status5;
  int pid;
  char ipstr[INET6_ADDRSTRLEN];
  struct sockaddr_storage their_storage;
  socklen_t their_len = sizeof (their_storage);

  /* cite Beej's socket programming guide pages 15/16 */
  memset (&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  for (int i=0; i < 5; i++)
  {
    if ((pid = fork()) == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

    const char* follower_udp_port;

    if (pid == 0)
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
      if ((status = getaddrinfo("nunki.usc.edu", follower_udp_port, &hints, 
                                &serv_info)) != 0) 
      {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit (EXIT_FAILURE);
      } 

      sock_fd = socket (serv_info->ai_family, serv_info->ai_socktype, 
                        serv_info->ai_protocol);

      if (sock_fd == SYSCALL_FAILURE)
      {
        perror ("socket");
        exit (EXIT_FAILURE);
      }

      status = bind (sock_fd, serv_info->ai_addr, serv_info->ai_addrlen);

      /* convert IP address of the server socket */
      inet_ntop (serv_info->ai_family, 
                 get_in_addr((struct sockaddr *)serv_info->ai_addr), ipstr, 
                 sizeof(ipstr));

      sockaddr_in *s = (sockaddr_in *)serv_info->ai_addr;

      cout << follower_header << " has UDP port " << s->sin_port;
      cout << " and IP address " << ipstr << endl;

      char buf[MAX_DATA_SIZE];

      while(true) 
      {
      	int num_bytes = recvfrom (sock_fd, buf, MAX_DATA_SIZE-1, 0, 
                               (struct sockaddr *)&their_storage, &their_len);
      	if (num_bytes == -1)
      	{
          perror ("recv");
          exit (EXIT_FAILURE);
        }
        if (num_bytes > 0) {
          buf[num_bytes] = '\0';
          if (strcmp(buf, END_TRANSMISSION_MESSAGE) == 0) 
          {
          	//cout << "buf: " << buf << endl;
          	//cout << "end transmission for " << follower_header << endl;
          	break;
          }
          else if (strcmp(buf, TWEET_A_END_TRANSMISSION) == 0)
          {
            cout << follower_header << " has received <TweetA>" << endl;
          }
          else if (strcmp(buf, TWEET_B_END_TRANSMISSION) == 0)
          {
            cout << follower_header << " has received <TweetB>" << endl;
          }
          else if (strcmp(buf, TWEET_C_END_TRANSMISSION) == 0)
          {
            cout << follower_header << " has received <TweetC>" << endl;
          }
          else
          {
          	//cout << "buf: " << buf << endl;
          }
        }
      }

      /* Close the UDP socket connection to the Server because no longer needed */
      if ((status = close (sock_fd)) == SYSCALL_FAILURE)
      {
        perror ("close");
        exit (EXIT_FAILURE);
      }

      freeaddrinfo (serv_info);  

      exit (EXIT_SUCCESS);
    }
  }

  wait (&udp_status1);
  wait (&udp_status2);
  wait (&udp_status3);
  wait (&udp_status4);
  wait (&udp_status5);

  return;
}

void Follower::Connect ()
{
  pid_t pid;
  char ipstr[INET6_ADDRSTRLEN];
  int len;
  int result;
  int tcp_status1, tcp_status2, tcp_status3, tcp_status4, tcp_status5;

  /* cite Beej's socket programming guide pages 15/16 */
  memset (&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  /* cite Beej's socket programming guide pages 15/16 */
  if ((result = getaddrinfo("nunki.usc.edu", SERVER_TCP_PORT, &hints, 
                            &serv_info)) != 0) 
  {
    fprintf (stderr, "getaddrinfo: %s\n", gai_strerror(result));
    exit (EXIT_FAILURE);
  }

  /* need to call fork three times for three Tweet people */
  for (int i=0; i < 5; i++)
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
          following = parseFollowing("Follower1.txt");
          likes = parseLikes("Follower1.txt");
          follower_header = FOLLOWER_1_HEADER;
          break;
        case 1:
          following = parseFollowing("Follower2.txt");
          likes = parseLikes("Follower2.txt");
          follower_header = FOLLOWER_2_HEADER;
          break;
        case 2:
          following = parseFollowing("Follower3.txt");
          likes = parseLikes("Follower3.txt");
          follower_header = FOLLOWER_3_HEADER;
          break;
        case 3:
          following = parseFollowing("Follower4.txt");
          likes = parseLikes("Follower4.txt");
          follower_header = FOLLOWER_4_HEADER;
          break;
        case 4:
          following = parseFollowing("Follower5.txt");
          likes = parseLikes("Follower5.txt");
          follower_header = FOLLOWER_5_HEADER;
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

       /* each Follower should connect with the Server over this TCP port */
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
      cout << follower_header << " has TCP port " << s.sin_port;
      cout << " and IP address " << ipstr << endl;

      /* send the header before sending the feedback so the Server knows how to 
         store them */
      string header = follower_header + "\n";
      result = send (sock_fd, header.c_str(), strlen(header.c_str()), 0);
      if (result == SYSCALL_FAILURE)
      {
        perror ("send");
        exit (EXIT_FAILURE);
      }

      /* send over all this Follower's Tweets that it follows from its input 
         file */
      for (int j=0; j < following.size(); j++)
      {
        string payload = "FOLLOW:" + following[j] + "\n";

        result = send (sock_fd, payload.c_str(), strlen(payload.c_str()), 0);
        if (result == SYSCALL_FAILURE)
        {
          perror ("send");
          exit (EXIT_FAILURE);
        }
      }

      /* send over all this Follower's feedback (likes) */
      for (int k=0; k < likes.size(); k++)
      {
        string payload = "LIKE:" + likes[k] + "\n";

        result = send (sock_fd, payload.c_str(), strlen(payload.c_str()), 0);
        if (result == SYSCALL_FAILURE)
        {
          perror ("send");
          exit (EXIT_FAILURE);
        }
      }

      cout << "Completed sending feedback for " << follower_header << endl;

      cout << "End of phase 2 for " << follower_header << endl;
      exit (EXIT_SUCCESS);
    }
  }

  /* wait for all three child processes to finish initial TCP communications with
     Server before parent process exits */
  wait (&tcp_status1);
  wait (&tcp_status2);
  wait (&tcp_status3);
  wait (&tcp_status4);
  wait (&tcp_status5);
}

/* CITE: Beej's programming guide page 28 
   get sockaddr, IPv4 or IPv6: */
void *Follower::get_in_addr (struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) 
  {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main ()
{
  Follower follower;

  follower.Receive();
  follower.Connect();

  return 0;
}

vector<string> parseFollowing (string file_name)
{
  ifstream ifs;
  string line;
  vector<string> following;

  ifs.open (file_name.c_str());

  if (!ifs.is_open())
  {
    cout << "could not open file: " << file_name << endl;
    return following;
  }   

   while (getline(ifs, line))
  {
    if (line.empty())
    {
      continue;
    }
    if (line.substr(0, 9).compare("Following") == 0) {
      //cout << line << endl;
      string contents = line.substr(10);
      stringstream ss(contents);
      string tweet;
      while (getline(ss, tweet, ',')) 
      {
        following.push_back(tweet);
      }
      //cout << contents << endl;
    }
  }
  /*
  for (unsigned int i=0; i < following.size(); i++) {
    cout << "following[" << i << "]: " << following[i] << endl;
  }
  */
  ifs.close ();
  return following;
}

vector<string> parseLikes (string file_name)
{
  ifstream ifs;
  string line;
  vector<string> likes;

  ifs.open (file_name.c_str());

  if (!ifs.is_open()) 
  {
    cout << "could not open file: " << file_name << endl;
    return likes;
  }

  while (getline(ifs, line))
  {
    if (line.empty()) 
    {
      continue;
    }
    if (line.substr(0, 6).compare("TweetA") == 0) 
    {
      //cout << line << endl;
      string like = line.substr(7);
      if (like.compare("like") == 0) 
      {
        likes.push_back(line.substr(0,6));
      }
    }
    else if (line.substr(0,6).compare("TweetB") == 0)
    {
      //cout << line << endl;
      string like = line.substr(7);
      if (like.compare("like") == 0) 
      {
        likes.push_back(line.substr(0,6));
      }
    }
    else if (line.substr(0,6).compare("TweetC") == 0)
    {
      //cout << line << endl;
      string like = line.substr(7);
      if (like.compare("like") == 0) 
      {
        likes.push_back(line.substr(0,6));
      }
    }
  }

  /*
  for (unsigned int i=0; i < likes.size(); i++) {
    cout << "likes[" << i << "]: " << likes[i] << endl;
  }
  */

  ifs.close ();
  return likes;
}

