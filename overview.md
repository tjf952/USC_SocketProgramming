
### Assignment: 

- I have completed the optional part (suffix) as well as all the original requirements
	- Include prefix and suffix from backendA, backendB, backendC
- Completed makefile that uses commands
	- make serverA
	- make serverB
	- make serverC
	- make aws
	- make monitor

### Code Files:

- aws.cpp
	- Main server that establishes TCP connections with client and monitor for communication. Also establishes UDP connections with serverA, serverB, and serverC for further back-end communication.
- client.cpp
	- Client front-end that allows a user to input commands (search, prefix, suffix) to receive a result from aws. Displays simple results received from aws through TCP connection.
- monitor.cpp
	- Displays extended results from client's request. Receives results from aws through TCP connection.
- serverA.cpp/serverB.cpp/serverC.cpp
	- Back-end servers that represent backendA.txt, backendB.txt, and backendC.txt. Receives request from aws through UDP connection and sends back results to be processed by aws.

### Message Format:

Idiosyncrasy 
- Program Fails if:
	- The program works as intended to by the instructions
- Program Exits If:
	- user input for client is incorrect in format
	- user input for ***function*** is not supported
	- user input for ***input*** is more than 27 characters
	- getaddrinfo() fails
	- invalid connection to socket
	- recv() has an error
	- send() has an error
	- recvfrom() has an error
	- sendto() has an error
	- listen() fails

### Sources:

#### "Beej's Guide to C Programming and Network Programming (socket programming)"
> 	- Section 6: Client-Server Background
> 		- 6.1. A Simple Stream Server (Used for reference in aws)
> 		- 6.2. A Simple Stream Client (Used for reference in client, aws)
> 		- 6.3. Datagram Sockets (Used for reference in aws, serverA, serverB, serverC)
> 	- Section 9: Man Pages (Used for reference in method and function calling)
> 	
> 	Overall, I drew the procedure of receiving and sending messages by TCP or UDP from this guide. I read over it in depth and used the information to form a working network of servers and clients. Functions include a wide array over all files such as:
> 	- getaddrinfo(), socket(), bind(), connect(), listen(), accept(), send(), recv(), sendto(), recvfrom(), close()

Text Editor: Sublime Text Version 3.0, Build 3143