#CC:=clang++
CC:=g++
SRC_DIR:= src
INC_DIR:= inc
OBJ_DIR:= obj
BIN_DIR:= bin
LIB_DIR:= lib
MAINS:= main.o
TEST_DIR:= tests
INCLUDES:= -Iinc
#INCLUDES+= $(addprefix -I,$(wildcard $(dir $(INC_DIR)/*/*.h)))
#CFLAGS:= -Werror -Wall -pedantic -pedantic-errors -Wextra -g -std=c++11 $(INCLUDES)
CPPFLAGS_DEBUG:= -O0 -g -D_DEBUG
CPPFLAGS:= -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -std=c++11 $(INCLUDES)
LDFLAGS:= `llvm-config --ldflags`
LDFLAGS+= -v -g -O0
SOURCES:= $(wildcard $(SRC_DIR)/*/*.cpp)
SOURCES+= src/main.cpp
OBJECTS:= $(addprefix $(OBJ_DIR)/,$(notdir $(SOURCES:.cpp=.o)))

LLVM_MODULES:= core mcjit native
LIBS:= `llvm-config --libs $(LLVM_MODULES)`
LIBS+= -lpthread -lffi -ldl -lm -lz -ltinfo

EXECUTABLE:= $(BIN_DIR)/demi

.PHONY: test all debug

all: debug

debug: CPPFLAGS+=$(CPPFLAGS_DEBUG)
debug: $(EXECUTABLE)

no-debug: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/*/%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) -c -o $@ $<

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) -c -o $@ src/main.cpp

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)

test:
	@echo $(SOURCES)
	@echo $(OBJECTS)
	@echo $(INCLUDES)
