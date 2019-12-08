main: main.cpp
	g++ -Wall -fexceptions -g  -c main.cpp -o main.o
	g++  -o ims main.o  -lsimlib -lm 
