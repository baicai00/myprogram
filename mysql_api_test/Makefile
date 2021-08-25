TARGET=test
CPPFLAGS=-g -O0 -std=c++11
CC=g++

APP_SRC=$(wildcard *.cpp)
APP_INC=-I./ -I/usr/local/mysql/include/

-include $(APP_SRC:.cpp=.d)
APP_OBJ=$(APP_SRC:.cpp=.o)

%.d: %.cpp
	$(CC) -MM $(CPPFLAGS) $(APP_INC) $< -o $@

%.o: %.cpp %.d
	$(CC) -c $(CPPFLAGS) $(APP_INC) $< -o $@

$(TARGET): $(APP_OBJ)
	$(CC) -o $@ $(CPPFLAGS) $(APP_OBJ)

clean:
	rm -rf *.o *.d
	rm -rf $(TARGET)