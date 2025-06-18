# --- Compiler and Flags --- 
CXX = g++
CXXFLAGS = -g -Wall -O3 -std=c++17
INCLUDES = -I/usr/include -I/usr/include/cppconn -I./include -I./include/external
LDFLAGS = -L/usr/lib/x86_64-linux-gnu
LDLIBS = -lmysqlcppconn

# --- Directories ---
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build
BIN_DIR = bin
SERVER_DIR = server

# --- Target Executable --- 
SERVER_EXE = $(BIN_DIR)/server
TEST_WITH_POOL_EXE = $(BIN_DIR)/test_with_pool
TEST_WITHOUT_POOL_EXE = $(BIN_DIR)/test_without_pool

# --- Source Files ---
SRC_FILES = $(wildcard $(SRC_DIR)/*.cc)
SERVER_FILES = $(wildcard $(SERVER_DIR)/*.cc)
TEST_FILES = $(wildcard $(TEST_DIR)/*.cc)

# --- Object Files --- 
SRC_OBJS = $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SRC_FILES))
SERVER_OBJS = $(patsubst $(SERVER_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SERVER_FILES))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cc, $(BUILD_DIR)/%.o,$(TEST_FILES))


# --- Rule to build .o from src/ and server/ and tests/ ---
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: $(SERVER_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Build object files from tests
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Rule to build the server ---
$(SERVER_EXE): $(SRC_OBJS) $(SERVER_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# --- Rule to build the test 1 ---
$(TEST_WITH_POOL_EXE): $(SRC_OBJS) $(BUILD_DIR)/test_with_pool.o | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# --- Rule to build the test 2 ---
$(TEST_WITHOUT_POOL_EXE): $(SRC_OBJS) $(BUILD_DIR)/test_without_pool.o | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# --- Create folders if needed --- 
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Phony targets
.PHONY: all clean test

# --- Default Target ---
all: $(SERVER_EXE) $(TEST_WITH_POOL_EXE) $(TEST_WITHOUT_POOL_EXE)

run: $(SERVER_EXE)
	cd $(BIN_DIR) && ./server

tests: $(TEST_WITH_POOL_EXE) $(TEST_WITHOUT_POOL_EXE)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Help
help:
	@echo "Usage:"
	@echo "  make              - Build main program"
	@echo "  make run          - Run main program"
	@echo "  make tests        - Build test programs"
	@echo "  make clean        - Remove all build files"