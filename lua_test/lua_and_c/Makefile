TARGET=main
CFLAGS=-g -O0
CC=gcc

APP_SRC=$(wildcard *.c)
APP_INC=-I./
APP_OBJ=$(APP_SRC:.C=.O)

%d: %c
	$(CC) -MM $(CFLAGS) $(APP_INC) $< -o $@

%.o: %.c %.d
	$(CC) -c $(CFLAGS) $(APP_INC) $< -o $@

$(TARGET): $(APP_OBJ)
	$(CC) -o $@ $(CFLAGS) $(APP_OBJ)

clean:
	rm -rf *.o
	rm -rf $(TARGET)
