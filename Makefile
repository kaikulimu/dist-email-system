CC=g++
LD=g++
CFLAGS=-g -Wall -std=c++11 -O2
CPPFLAGS=-I. -I./include
SP_LIBRARY_DIR=.
DEP=header.o logger.o AntiEntropyMachine.o clientSpreadUtil.o spreadUtil.o message.o serverSpreadUtil.o mystring.o


all: server client

%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

server:  $(SP_LIBRARY_DIR)/libspread-core.a $(DEP) server.o
	$(LD) -o $@ server.o $(DEP) $(SP_LIBRARY_DIR)/libspread-core.a -ldl -lm -lrt -lnsl $(SP_LIBRARY_DIR)/libspread-util.a

client:  $(SP_LIBRARY_DIR)/libspread-core.a client.o $(DEP)
	$(LD) -o $@ client.o $(DEP) $(SP_LIBRARY_DIR)/libspread-core.a -ldl -lm -lrt -lnsl $(SP_LIBRARY_DIR)/libspread-util.a

clean:
	(rm -rf *.o *.out *.dSYM server client test *.exe *.stackdump 0 1 2 3 4 5)

cleanlog:
	(rm -rf 0 1 2 3 4 5)

test:
	$(CC) $(CFLAGS) unit_test.cpp mystring.cpp message.cpp logger.cpp -o test && ./test

ups:
	make clean && scp -r ./* slin52@ugrad1.cs.jhu.edu:~/437/test/
upv:
	make clean && scp -r ./* vyan1@ugrad1.cs.jhu.edu:~/disSys/final/

s1:
	reset && clear && ./server 1

s2:
	reset && clear && ./server 2

s3:
	reset && clear && ./server 3
s4:
	reset && clear && ./server 4
s5:
	reset && clear && ./server 5

