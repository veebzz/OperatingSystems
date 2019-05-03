C = gcc
CFLAGS = -Wall -g -w -std=c11 -lm -lpthread -lrt
TARGET1 = oss
OBJ1 = oss.c
TARGET2 = user
OBJ2 = user.c
all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJ1)
$(TARGET2): $(OBJ2)
	$(CC) $(CFLAGS) -o $(TARGET2) $(OBJ2)

oss.o: oss.c
	$(CC) $(CFLAGS) -c oss.c
user.o: user.c:
	$(CC) $(CFLAGS) -c user.c


clean:
	/bin/rm -f *.o log.txt all