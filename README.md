# Socket Programming

### Description
This project uses a basic client/server architecture and network programming techniques to simulate Twitter. Tweeters 
tweet to a server by opening up TCP connections, and the server then communicates these tweets to a Tweeters followers
over UDP sockets. The followers then provide feedback to the server (by potentially liking some tweets) using TCP,
and the server relays this on to the Tweeters via TCP. A more detailed description of the project can be found in 
ProjectDescription.pdf

 