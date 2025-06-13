CXX = g++
CXXFLAGS = -g -Wall -std=c++17
INCLUDES = -I/usr/include -I/usr/include/cppconn -I./include
LDFLAGS = -L/usr/lib/x86_64-linux-gnu
LDLIBS = -lmysqlcppconn

SRC_DIR = ./src
BUILD_DIR = ./build
BIN_DIR = ./bin
TARGET = $(BIN_DIR)/baby-dbcp

SRCS = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(BUILD_DIR) $(BIN_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(TARGET)