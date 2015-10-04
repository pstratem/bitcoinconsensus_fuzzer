run_script: run_script.cpp
	afl-g++ -ggdb -O2 -Wall -Werror -pedantic --std=c++11 -o run_script run_script.cpp -lssl -lcrypto -lstdc++ -lbitcoinconsensus -lboost_system -lsecp256k1 -lboost_asio
