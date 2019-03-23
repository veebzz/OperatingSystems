C = gcc
CFLAGS = -Wall -g -w -pthread
TARGET1 = master
OBJ1 = master.c
TARGET2 = palin
OBJ2 = palin.c
all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJ1)
$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJ2)

master.o: master.c
	$(CC) $(CFLAGS) -c master.c
palin.o: user.c
	$(CC) $(CFLAGS) -c palin.c


clean:
	/bin/rm -f *.o nopalin.out palin.out  $(TARGET1) $(TARGET2)
