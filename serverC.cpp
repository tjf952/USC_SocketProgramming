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

#define CPORT "23982"
#define UDPPORT "24982"
#define MAXDATASIZE 2048

using namespace std;

map<string, string> dictionary;
map<string, string>::iterator it;

string search(string str){
	string results = "", def1 = "", sim = "", def2 = "", d = " :: ";
	int num = 0, cnt = 0;
	for(it=dictionary.begin(); it!=dictionary.end(); ++it){
		string word = it->first;
		int c = 0;
		if(str.length() != word.length()) continue;
		for(unsigned int i = 0; i < str.length(); ++i){
			if(tolower(str[i]) != tolower(word[i])) ++c;
		}
		if(c == 0){
			num = 1;
			def1 = it->second;
		} else if(c == 1){
			++cnt;
			sim = word;
			def2 = it->second;
		}
	}
	cout << "The ServerC has found < " << num << 
		" > match and < " << cnt << " > similar words" << endl;
	results += to_string(num) + d + def1 + d + to_string(cnt) + d + sim + d + def2;
	return results;
}

string prefix(string str){
	string results = "", words = "", d = " :: ";
	int cnt = 0;
	for(it=dictionary.begin(); it!=dictionary.end(); ++it){
		string word = it->first;
		int c = 0;
		if(str.length() > word.length()) continue;
		for(unsigned int i = 0; i < str.length(); ++i){
			if(tolower(str[i]) != tolower(word[i])) ++c;
		}
		if(c == 0){
			words += d + word;
			++cnt;
		}
	}
	cout << "The ServerC has found < " << cnt << " > matches" << endl;
	results += to_string(cnt) + words;
	return results;
}

string suffix(string str){
	string results = "", words = "", d = " :: ";
	int cnt = 0;
	for(it=dictionary.begin(); it!=dictionary.end(); ++it){
		string word = it->first;
		int c = 0;
		if(str.length() > word.length()) continue;
		for(unsigned int i = 0; i < str.length(); ++i){
			if(tolower(str[str.length()-i-1]) != tolower(word[word.length()-i-1])) ++c;
		}
		if(c == 0){
			words += d + word;
			++cnt;
		}
	}
	cout << "The ServerC has found < " << cnt << " > matches" << endl;
	results += to_string(cnt) + words;
	return results;
}

string func(string function, string input){
	string results;
	if(function == "search"){
		results = function + " :: " + search(input);
	} else if(function == "prefix"){
		results = function + " :: " + prefix(input);
	} else if(function == "suffix"){
		results = function + " :: " + suffix(input);
	} else {
		results = "";
	}
	return results;
}

bool parseFile(const string &filename){
	// Opens file and prepares for transfer
	ifstream infile(filename.c_str());
	if(!infile) return false;
	string line, word, definition;
	string delimiter = " :: ";

	// Reads from file into map
	while(getline(infile, line)){
		size_t pos = line.find(delimiter);
		word = line.substr(0, pos);
		line.erase(0, pos+delimiter.length());
		definition = line;
		dictionary.insert(pair<string, string>(word, definition));
	}
	infile.close();
	return true;
}

int main(void)
{
	int sockfd, sockfdU, byteNum, error;
	char buffer[MAXDATASIZE];
	struct addrinfo serv_info, *servinfo, *process;
	struct addrinfo serv_infoU, *servinfoU, *processU;
	struct sockaddr_storage curr_addr;
	socklen_t addr_len;

	// Parse dictionary
	if(!parseFile("backendC.txt")){
		exit(0);
		perror("file failed");
	}

	// Sets server classification
	memset(&serv_info, 0, sizeof serv_info);
	serv_info.ai_family = AF_INET;
	serv_info.ai_socktype = SOCK_DGRAM;
	serv_info.ai_flags = AI_PASSIVE;
	memset(&serv_infoU, 0, sizeof serv_infoU);
	serv_infoU.ai_family = AF_INET;
	serv_infoU.ai_socktype = SOCK_DGRAM;
	serv_infoU.ai_flags = AI_PASSIVE;

	// Gets server information 
	if(( error = getaddrinfo("localhost", CPORT, &serv_info, &servinfo) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}
	if(( error = getaddrinfo("localhost", UDPPORT, &serv_infoU, &servinfoU) ) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(1);
	}

	// Find usable process for ports
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
	for(processU = servinfoU; processU != NULL; processU = processU->ai_next){
		if((sockfdU = socket(processU->ai_family, processU->ai_socktype, processU->ai_protocol)) == -1){
			perror("socket failed");
			continue;
		}
		break;
	}

	// Check if valid connection
	if((process == NULL) || (processU == NULL)){
		fprintf(stderr, "failed bind\n");
		exit(1);
	}
	freeaddrinfo(servinfo);
	freeaddrinfo(servinfoU);

	cout << "\nThe ServerC is up and running using UDP on port " << CPORT << "." << endl;

	// Loop for receiving datagrams
	while(1){
		// Blocks until datagram received
		addr_len = sizeof curr_addr;
		bzero(buffer, MAXDATASIZE);
		if((byteNum = recvfrom(sockfd, buffer, MAXDATASIZE-1, 0,
			(struct sockaddr*)&curr_addr, &addr_len)) == -1){
			perror("receive failed");
			exit(1);
		}

		// Decode initial message
		string msg = string(buffer);
		string delimiter = " :: ";
		size_t pos = msg.find(delimiter);
		string function = msg.substr(0, pos);
		msg.erase(0, pos+delimiter.length());
		string input = msg;
		cout << "The ServerC received input < " << input << " > and operation < " 
			<< function << " >" << endl;

		// Get results in a string
		string temp = "C :: " + func(function, input);

		// Send back to aws
		const char* msg2 = temp.c_str();
		if ((byteNum = sendto(sockfdU, msg2, strlen(msg2), 0, processU->ai_addr, processU->ai_addrlen)) == -1) {
			perror("send failed");
			exit(1);
		}
		
		cout << "The ServerC finished sending the output to AWS\n" << endl;
	}

	close(sockfd);
	close(sockfdU);
	return 0;
}