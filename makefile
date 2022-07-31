CC=g++
CXXFLAGS=-std=c++0x
CFLAGS=-I
skiplist: main.o
	$(CC) -o ./bin/main main.p --std=c++11 -lpthread
	rm -f ./*.o
clean:
	rm -f ./*.o