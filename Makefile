CXX = g++
CPPFLAGS = -g -Wall -std=c++11
BIN_DIR = bin

all:  serverA serverB serverC aws client monitor

serverA: serverA.cpp $(BIN_DIR)/.dirstamp
ifeq "$(MAKECMDGOALS)" "all"
	$(CXX) $(CPPFLAGS) serverA.cpp -o $(BIN_DIR)/serverA
else
	./$(BIN_DIR)/serverA
endif

serverB: serverB.cpp $(BIN_DIR)/.dirstamp
ifeq "$(MAKECMDGOALS)" "all"
	$(CXX) $(CPPFLAGS) serverB.cpp -o $(BIN_DIR)/serverB
else
	./$(BIN_DIR)/serverB
endif

serverC: serverC.cpp $(BIN_DIR)/.dirstamp
ifeq "$(MAKECMDGOALS)" "all"
	$(CXX) $(CPPFLAGS) serverC.cpp -o $(BIN_DIR)/serverC
else
	./$(BIN_DIR)/serverC
endif

aws: aws.cpp $(BIN_DIR)/.dirstamp
ifeq "$(MAKECMDGOALS)" "all"
	$(CXX) $(CPPFLAGS) aws.cpp -o $(BIN_DIR)/aws
else
	./$(BIN_DIR)/aws
endif

monitor: monitor.cpp $(BIN_DIR)/.dirstamp
ifeq "$(MAKECMDGOALS)" "all"
	$(CXX) $(CPPFLAGS) monitor.cpp -o $(BIN_DIR)/monitor
else
	./$(BIN_DIR)/monitor
endif

client: client.cpp
ifeq "$(MAKECMDGOALS)" "all"
	$(CXX) $(CPPFLAGS) client.cpp -o client
endif

.PHONY: clean

clean:
	rm -rf $(BIN_DIR)

$(BIN_DIR)/.dirstamp:
	mkdir -p $(BIN_DIR)
	touch $(BIN_DIR)/.dirstamp			