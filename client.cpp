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

#define PORT "25982"
#define MAXDATASIZE 2048

using namespace std;

void* get_inaddr(struct sockaddr *sa){ 
	return &(((struct sockaddr_in*)sa)->sin_addr);
}

void search(string input, vector<string> words){
	if(words[1] == "0"){
		cout << "Found no matches for < " << input << " >\n" << endl;
	} else {
		cout << "Found a match for < " << input << " >:\n < " << words[2] << " >\n" << endl;
	}
}

void fix(string input, vector<string> words){
	if(words[1] == "0"){
		cout << "Found no matches for < " << input << " >\n" << endl;
	} else {
		cout << "Found < " << words[1] << " > matches for < " << input << " >:\n";
		for(unsigned int i = 2; i < words.size(); ++i){
			cout << "< " << words[i] << " >\n";
		}
		cout << endl;
	}
}

void decode(string input, char* buffer){
	vector<string> words;
	string line = string(buffer);
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

    // Depending on function, prints specific
    if(words[0] == "search"){
		search(input, words);
	} else if((words[0] == "prefix") || (words[0] == "suffix")){
		fix(input, words);
	} else {
		perror("format failed");
	}

}

int main(int argc, char* argv[])
{
	int sockfd, byteNum, error;
	char buffer[MAXDATASIZE];
	struct addrinfo serv_info, *servinfo, *process;

	// Checks for correct argv
	if(argc != 3){
		fprintf(stderr, "usage: ./client <function> <input>\n");
		exit(1);
	}
	string check = argv[1];
	if((check != "search") && (check != "prefix") && (check != "suffix")){
		fprintf(stderr, "function: enter 'search' or 'prefix' or 'suffix'\n");
		exit(1);
	}
	string check2 = argv[2];
	if(check2.size() > 27){
		fprintf(stderr, "input: must be <= 27 characters\n");
		exit(1);
	}

	// Sets server classification
	memset(&serv_info, 0, sizeof serv_info);
	serv_info.ai_family = AF_INET;
	serv_info.ai_socktype = SOCK_STREAM;

	// Gets server information from aws
	if(( error = getaddrinfo("localhost", PORT, &serv_info, &servinfo) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}

	// Find usable process for client port
	for(process = servinfo; process != NULL; process = process->ai_next){
		if((sockfd = socket(process->ai_family, process->ai_socktype, process->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		if(connect(sockfd, process->ai_addr, process->ai_addrlen) == -1){
			close(sockfd);
			perror("connection failed");
			continue;
		}
		break;
	}

	// Check if valid connection
	if(process == NULL){
		fprintf(stderr, "invalid connection\n");
		exit(1);
	}
	cout << "\nThe client is up and running." << endl;
	freeaddrinfo(servinfo);

	// Prepares msg data
	bzero(buffer, MAXDATASIZE);
	string temp = "";
	temp += argv[1];
	temp += " ";
	temp += argv[2];
	const char* msg = temp.c_str(); 

	// Sends client code to aws
	if(send(sockfd, msg, strlen(msg), 0) == -1) perror("send failed");
	cout << "The client sent < " << argv[2] << " > and < " << argv[1] << " > to AWS." << endl;

	// Waits to receive response
	if((byteNum = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1){
		perror("recv failed");
		exit(1);
	}

	// Decodes msg to print
	string input = argv[2];
	decode(input, buffer);

    close(sockfd);
    return 0;
}