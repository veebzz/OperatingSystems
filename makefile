#Vibhav Chemarla
#CS4760 PA 1
#01/28/19

CC = gcc
 CFLAGS = -Wall -g
 TARGET = syscall
 OBJS = main.o
 $(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c


clean:
	/bin/rm -f *.o output.dat $(TARGET)
