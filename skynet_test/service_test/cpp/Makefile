include ../../Make.rule

CPP_FILES+=$(wildcard *.cpp)
C_FILES+=$(wildcard *.c)
H_FILES+=$(wildcard *.h)
INC+=-I./
LIBS += --share

OBJ_DIR = ./obj/
OBJ := $(patsubst %.cpp, $(OBJ_DIR)%.o, $(CPP_FILES)) $(patsubst %.c, $(OBJ_DIR)%.o, $(C_FILES))

BIN = watchdog.so
BIN_PATH=../service/
TARGET = $(BIN_PATH)$(BIN)

$(TARGET):$(OBJ) $(H_FILES)
	@echo link...  $(TARGET)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(INC) $(LIBS)
	
$(OBJ_DIR)%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< $(INC)
	
$(OBJ_DIR)%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INC)

clean:
	rm -f $(OBJ_DIR)* $(TARGET)