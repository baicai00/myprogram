TARGET=beelog.so
CFLAGS=-g -O0 -shared -fPIC
CC=g++

APP_SRC=$(wildcard *.cpp)
APP_INC=-I./
APP_OBJ=$(APP_SRC:.CPP=.O)
APP_LIB=-L/usr/local/lib\
		-lglog -llua -lm -ldl
		
%d: %cpp
	$(CC) -MM $(CFLAGS) $(APP_INC) $< -o $@

$(TARGET): $(APP_OBJ)
	$(CC) -o $@ $(CFLAGS) $(APP_OBJ) $(APP_LIB)

clean:
	rm -rf *.o
	rm -rf $(TARGET)