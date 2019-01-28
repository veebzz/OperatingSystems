#Vibhav Chemarla
#CS4760 PA 1
#01/28/19

CC = gcc
 TARGET = syscall
 OBJS = main.o
 $(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

main.o: main.c
	$(CC) -c main.c


clean:
	/bin/rm -f *.o $(TARGET)
