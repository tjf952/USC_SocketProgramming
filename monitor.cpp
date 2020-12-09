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

#define PORT "26982"
#define MAXDATASIZE 2048

using namespace std;

void* get_inaddr(struct sockaddr *sa){ 
	return &(((struct sockaddr_in*)sa)->sin_addr);
}

void search(vector<string> words){
	if(words[2] == "0"){
		cout << "Found no matches for < " << words[0] << " >\n" << endl;
	} else {
		cout << "Found a match for < " << words[0] << " >:\n < " << words[3] << " >" << endl;
		if(words[4] == "0"){
			cout << "Found no similar words for < " << words[0] << " >\n" << endl;
		} else {
			cout << "One edit distance match is < " << words[5] << " >:\n <" << words[6] << " >\n" << endl;
		}
	}
}

void fix(vector<string> words){
	if(words[2] == "0"){
		cout << "Found no matches for < " << words[0] << " >\n" << endl;
	} else {
		cout << "Found < " << words[2] << " > matches for < " << words[0] << " >:\n";
		for(unsigned int i = 3; i < words.size(); ++i){
			cout << "< " << words[i] << " >\n";
		}
		cout << endl;
	}
}

void decode(char* buffer){
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
    if(words[1] == "search"){
		search(words);
	} else if((words[1] == "prefix") || (words[1] == "suffix")){
		fix(words);
	} else {
		perror("format failed");
	}

}

int main(void)
{
	int sockfd, byteNum, error;
	char buffer[MAXDATASIZE];
	struct addrinfo serv_info, *servinfo, *process;

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
	cout << "\nThe Monitor is up and running." << endl;
	freeaddrinfo(servinfo);

	while(1){
		// Waits to receive response
		bzero(buffer, MAXDATASIZE);
		if((byteNum = recv(sockfd, buffer, MAXDATASIZE-1, 0)) == -1){
			perror("recv failed");
			exit(1);
		}

		// Decodes msg to print
		decode(buffer);
	}

    close(sockfd);
    return 0;
}