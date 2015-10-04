run_script: run_script.cpp
	afl-g++ -ggdb -O2 -Wall -Werror -pedantic --std=c++11 -o run_script run_script.cpp -lbitcoinconsensus
