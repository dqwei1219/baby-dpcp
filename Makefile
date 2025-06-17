CXX = g++
CXXFLAGS = -g -Wall -O3 -std=c++17
INCLUDES = -I/usr/include -I/usr/include/cppconn -I./include
LDFLAGS = -L/usr/lib/x86_64-linux-gnu
LDLIBS = -lmysqlcppconn

# Directories
SRC_DIR = ./src
TEST_DIR = ./tests
BUILD_DIR = ./build
BIN_DIR = ./bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRCS))

# Specific object files we need
CONNECTION_OBJ = $(BUILD_DIR)/Connection.o
POOL_OBJ = $(BUILD_DIR)/CommonConnectionPool.o

# Targets
TARGET = $(BIN_DIR)/baby-dbcp
TEST_WITHOUT_POOL = $(BIN_DIR)/test_without_pool
TEST_WITH_POOL = $(BIN_DIR)/test_with_pool

# Default target
all: $(TARGET)

# Main program
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Build any object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Test without pool (only needs Connection.o)
$(TEST_WITHOUT_POOL): $(TEST_DIR)/test_without_pool.cc $(CONNECTION_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Test with pool (needs both objects)
$(TEST_WITH_POOL): $(TEST_DIR)/test_with_pool.cc $(CONNECTION_OBJ) $(POOL_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Phony targets
.PHONY: all clean test run run-tests

# Build all tests
test: $(TEST_WITHOUT_POOL) $(TEST_WITH_POOL)

# Run main program
run: $(TARGET)
	./$(TARGET)

# Run all tests
run-tests: test
	@./$(TEST_WITHOUT_POOL)
	@./$(TEST_WITH_POOL)

# Clean everything
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Help
help:
	@echo "Usage:"
	@echo "  make              - Build main program"
	@echo "  make test         - Build test programs"
	@echo "  make run          - Run main program"
	@echo "  make run-tests    - Build and run all tests"
	@echo "  make clean        - Remove all build files"