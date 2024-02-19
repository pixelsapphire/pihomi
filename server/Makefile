CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -pedantic -O0 -g -DRASPBERRY
SRC = src
LIBS = -lwiringPi -lpigpiod_if2
BUILD = build
OBJ = build/obj
TARGET = build/main
SRC_FILES = $(wildcard $(SRC)/*.cpp)
OBJ_FILES = $(SRC_FILES:$(SRC)/%.cpp=$(OBJ)/%.o)

all: $(BUILD) $(OBJ) $(TARGET)

$(BUILD):
	mkdir -p $@

$(OBJ):
	mkdir -p $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

run: all
	sudo ./$(TARGET)

clean:
	rm -rf $(BUILD)
