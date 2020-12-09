#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <vector>

#include <signal.h>
#include <sys/time.h>

#define UDPPORT "24982"
#define CLIPORT "25982"
#define MONPORT "26982"
#define APORT "21982"
#define BPORT "22982"
#define CPORT "23982"
#define QUEUE 10
#define MAXDATASIZE 2048

using namespace std;

void* get_inaddr(struct sockaddr *sa){ 
	return &(((struct sockaddr_in*)sa)->sin_addr);
}

// For reaping zombie processes
void reaper(int s){
	int temp = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = temp;
}

void printResult(vector<string> words){
	// Depending on function, prints 
	if(words[1] == "search"){
		cout << "The AWS received < " << words[2] << " > similar words from Backend-Server < " <<
			words[0] << " > using UDP over port < " << UDPPORT << " >" << endl;
	} else if((words[1] == "prefix") || (words[1] == "suffix")){
		cout << "The AWS received < " << words[2] << " > matches from Backend-Server < " <<
			words[0] << " > using UDP over port < " << UDPPORT << " >" << endl;
	} else {
		perror("format function failed");
	}
}

vector<string> decode(string line){
	vector<string> words;
	string delimiter = " :: ", word;
	size_t pos = line.find(delimiter);

	// Puts each part in vector
	while((pos = line.find(delimiter)) != string::npos){
		word = line.substr(0, pos);
	    line.erase(0, pos+delimiter.length());
	    words.push_back(word);
	}
	word = line.substr(0, pos);
    words.push_back(word);

    // Print result msg
    return words;
}

vector<string> combine(vector<string> a, vector<string> b){
	// Checks if empty, if so deep copy vs combine
	if(a.size() == 0){
		for(unsigned int i = 1; i < b.size(); ++i){
			a.push_back(b[i]);
		}
	} else {
		if(a[0] == "search"){
			if(a[1] == "0"){
				a[1] = b[2];
				a[2] = b[3];
			}
			int sum = atoi(a[3].c_str()) + atoi(b[4].c_str());
			a[3] = to_string(sum);
		} else if((a[0] == "prefix") || (a[0] == "suffix")){
			int sum = atoi(a[1].c_str()) + atoi(b[2].c_str());
			a[1] = to_string(sum);
			for(unsigned int i = 3; i < b.size(); ++i){
				a.push_back(b[i]);
			}
		} else {
			perror("combine failed");
		}
	}
	return a;
}


string communicateUDP(string function, string input)
{
	int sockfd, sockfdA, sockfdB, sockfdC, byteNum, error;
	char buffer2[MAXDATASIZE];
	struct addrinfo serv_info, *servinfo, *process;
	struct addrinfo serv_infoA, *servinfoA, *processA;
	struct addrinfo serv_infoB, *servinfoB, *processB;
	struct addrinfo serv_infoC, *servinfoC, *processC;
	struct sockaddr_storage curr_addr;
	socklen_t addr_len;

	// Sets server classification
	memset(&serv_info, 0, sizeof serv_info);
	serv_info.ai_family = AF_INET;
	serv_info.ai_socktype = SOCK_DGRAM;
	serv_info.ai_flags = AI_PASSIVE;
	memset(&serv_infoA, 0, sizeof serv_infoA);
	serv_infoA.ai_family = AF_INET;
	serv_infoA.ai_socktype = SOCK_DGRAM;
	memset(&serv_infoB, 0, sizeof serv_infoB);
	serv_infoB.ai_family = AF_INET;
	serv_infoB.ai_socktype = SOCK_DGRAM;
	memset(&serv_infoC, 0, sizeof serv_infoC);
	serv_infoC.ai_family = AF_INET;
	serv_infoC.ai_socktype = SOCK_DGRAM;

	// Gets server information from aws
	if(( error = getaddrinfo("localhost", UDPPORT, &serv_info, &servinfo) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}
	if(( error = getaddrinfo("localhost", APORT, &serv_infoA, &servinfoA) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}
	if(( error = getaddrinfo("localhost", BPORT, &serv_infoB, &servinfoB) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}
	if(( error = getaddrinfo("localhost", CPORT, &serv_infoC, &servinfoC) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}

	// Find usable process for client port
	for(process = servinfo; process != NULL; process = process->ai_next){
		if((sockfd = socket(process->ai_family, process->ai_socktype, process->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		if(bind(sockfd, process->ai_addr, process->ai_addrlen) == -1){
			close(sockfd);
			perror("bind failed");
			continue;
		}
		break;
	}
	for(processA = servinfoA; processA != NULL; processA = processA->ai_next){
		if((sockfdA = socket(processA->ai_family, processA->ai_socktype, processA->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		break;
	}
	for(processB = servinfoB; processB != NULL; processB = processB->ai_next){
		if((sockfdB = socket(processB->ai_family, processB->ai_socktype, processB->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		break;
	}
	for(processC = servinfoC; processC != NULL; processC = processC->ai_next){
		if((sockfdC = socket(processC->ai_family, processC->ai_socktype, processC->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		break;
	}

	// Check if valid connection
	if((process == NULL) || (processA == NULL) || (processB == NULL) || (processC == NULL)){
		fprintf(stderr, "invalid connection\n");
		exit(1);
	}

	// Sends msg to server A, B, C
	string temp = function + " :: " + input;
	const char* msg = temp.c_str();
	if ((byteNum = sendto(sockfdA, msg, strlen(msg), 0, processA->ai_addr, processA->ai_addrlen)) == -1) {
		perror("send failed");
		exit(1);
	}
	cout << "The AWS sent < " << input << " > and < " << function << " > to Backend-Server A" << endl;
	if ((byteNum = sendto(sockfdB, msg, strlen(msg), 0, processB->ai_addr, processB->ai_addrlen)) == -1) {
		perror("send failed");
		exit(1);
	}
	cout << "The AWS sent < " << input << " > and < " << function << " > to Backend-Server B" << endl;
	if ((byteNum = sendto(sockfdC, msg, strlen(msg), 0, processC->ai_addr, processC->ai_addrlen)) == -1) {
		perror("send failed");
		exit(1);
	}
	cout << "The AWS sent < " << input << " > and < " << function << " > to Backend-Server C" << endl;

	// Receives msg from server A, B, C
	vector<string> results, vectorA, vectorB, vectorC;
	for(int i = 0; i < 3; ++i){
		// Blocks until datagram received
		addr_len = sizeof curr_addr;
		bzero(buffer2, MAXDATASIZE);
		if((byteNum = recvfrom(sockfd, buffer2, MAXDATASIZE-1, 0,
			(struct sockaddr*)&curr_addr, &addr_len)) == -1){
			perror("receive failed");
			exit(1);
		}

		// Decode message
		string msg2 = string(buffer2);
		vector<string> words = decode(msg2);
		if(words[0] == "A") vectorA = decode(msg2);
		else if(words[0] == "B") vectorB = decode(msg2);
		else if(words[0] == "C") vectorC = decode(msg2);

		// Edits results with each server msg
		results = combine(results, words);
	}

	// Print results
	printResult(vectorA);
	printResult(vectorB);
	printResult(vectorC);

	freeaddrinfo(servinfo);
	freeaddrinfo(servinfoA);
	freeaddrinfo(servinfoB);
	freeaddrinfo(servinfoC);

	close(sockfdA);
	close(sockfdB);
	close(sockfdC);

	// Message for client
	string result = results[0];
	for(unsigned int i = 1; i < results.size(); ++i){
		result += " :: " + results[i];
	}
	cout << "The AWS sent < " << results[1] << " > matches to client." << endl;

	// Message for monitor
	if(results[0] == "search"){
		cout << "The AWS sent < " << input << " > and < " << results[4] <<
			" > to the monitor via TCP port " << MONPORT << ".\n" << endl;
	} else if((results[0] == "prefix") || (results[0] == "suffix")){
		cout << "The AWS sent < " << results[1] << " > matches to the monitor via TCP port " <<
			MONPORT << ".\n" << endl;
	}

	// Returns final msg to be sent back to client and monitor
	return result;
}

void response(const char buffer3[], int currfd, int currfdM){
	// Decode initial message
	string msg = string(buffer3);
	string delimiter = " ";
	size_t pos = msg.find(delimiter);
	string function = msg.substr(0, pos);
	msg.erase(0, pos+delimiter.length());
	string input = msg;
	cout << "The AWS received input=< " << input << " > and function=< " << function <<
		" > from the client using TCP over port " << CLIPORT << endl;

	// Send and receive through UDP
	msg = communicateUDP(function, input);
	buffer3 = msg.c_str();

	// Send to client
	if(send(currfd, buffer3, strlen(buffer3), 0) == -1) perror("send failed");

	// Send to monitor
	string msg2 = input + " :: " + msg;
	buffer3 = msg2.c_str();
	if(send(currfdM, buffer3, strlen(buffer3), 0) == -1) perror("send failed");
}

int main(void)
{
	int sockfd, currfd, byteNum, error, a=1;
	char buffer[MAXDATASIZE];
	struct addrinfo serv_info, *servinfo, *process;
	struct sockaddr_storage curr_addr;
	struct sigaction sig;
	socklen_t sin_size;

	int sockfdM, currfdM, b=1;
	struct addrinfo serv_infoM, *servinfoM, *processM;
	struct sockaddr_storage curr_addrM;
	socklen_t sin_sizeM;

	// Sets server classification for client
	memset(&serv_info, 0, sizeof serv_info);
	serv_info.ai_family = AF_INET;
	serv_info.ai_socktype = SOCK_STREAM;
	serv_info.ai_flags = AI_PASSIVE;

	// Sets server classification for monitor
	memset(&serv_infoM, 0, sizeof serv_infoM);
	serv_info.ai_family = AF_INET;
	serv_info.ai_socktype = SOCK_STREAM;
	serv_info.ai_flags = AI_PASSIVE;

	// Gets server information from aws
	if(( error = getaddrinfo("localhost", CLIPORT, &serv_info, &servinfo) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}

	if(( error = getaddrinfo("localhost", MONPORT, &serv_infoM, &servinfoM) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}

	// Find usable process for client port
	for(process = servinfo; process != NULL; process = process->ai_next){
		if((sockfd = socket(process->ai_family, process->ai_socktype, process->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &a, sizeof(int)) == -1){
			perror("setsockopt failed");
			exit(1);
		}
		if(bind(sockfd, process->ai_addr, process->ai_addrlen) == -1){
			close(sockfd);
			perror("bind failed");
			continue;
		}
		break;
	}

	// Find usable process for monitor port
	for(processM = servinfoM; processM != NULL; processM = processM->ai_next){
		if((sockfdM = socket(processM->ai_family, processM->ai_socktype, processM->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		if(setsockopt(sockfdM, SOL_SOCKET, SO_REUSEADDR, &b, sizeof(int)) == -1){
			perror("setsockopt failed");
			exit(1);
		}
		if(bind(sockfdM, processM->ai_addr, processM->ai_addrlen) == -1){
			close(sockfdM);
			perror("bind failed");
			continue;
		}
		break;
	}

	// Check if valid connection
	if((process == NULL) || (processM == NULL)){
		fprintf(stderr, "failed bind\n");
		exit(1);
	}
	freeaddrinfo(servinfo);
	freeaddrinfo(servinfoM);

	cout << "\nThe AWS is up and running." << endl;

	// Listens for connections
	if(listen(sockfd, QUEUE) == -1){
		perror("listen failed");
		exit(1);
	}

	// Reaping process
	sig.sa_handler = reaper;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sig, NULL) == -1){
		perror("sigaction failed");
		exit(1);
	}

	// Receiving monitor port
	if(listen(sockfdM, QUEUE) == -1){
		perror("listen failed");
		exit(1);
	}
	sin_sizeM = sizeof curr_addrM;
	currfdM = accept(sockfdM, (struct sockaddr*)&curr_addrM, &sin_sizeM);
	if(currfdM == -1){
		perror("accept failed");
	}

	// Loop for accepting and responding to clients
	while(1){
		// Accepts client connection
		sin_size = sizeof curr_addr;
		currfd = accept(sockfd, (struct sockaddr*)&curr_addr, &sin_size);
		if(currfd == -1){
			perror("accept failed");
			continue;
		}

		// Recieves info and according sends
		bzero(buffer, MAXDATASIZE);
		if((byteNum = recv(currfd, buffer, MAXDATASIZE-1, 0)) == -1){
			perror("recv failed");
			exit(1);
		} 

		// Sends info back to client
		if(!fork()){
			close(sockfd);
			response(buffer, currfd, currfdM);
			close(currfd);
			exit(0);
		}
		close(currfd);
	}

	close(sockfdM);
	close(currfdM);
    return 0; 	
}