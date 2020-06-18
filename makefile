CXX = g++
AM_CXXFLAGS = -Wall -W -std=c++11
main.out:
	g++ -o main.out main.cpp -lncurses -std=c++11