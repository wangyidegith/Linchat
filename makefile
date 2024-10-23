# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Directories
SRC_DIR = src
LIB_DIR = lib
BUILD_DIR = build

# Source files for each target
INPUT2ALL_SRCS = $(SRC_DIR)/client/input2all.cpp $(SRC_DIR)/client/packet.cpp $(LIB_DIR)/m-net.cpp
INPUT2DSTNAME_SRCS = $(SRC_DIR)/client/input2dstname.cpp $(SRC_DIR)/client/packet.cpp
INPUT2SINGLE_SRCS = $(SRC_DIR)/client/input2single.cpp $(SRC_DIR)/client/packet.cpp $(LIB_DIR)/m-net.cpp
OUTPUT2ALL_SRCS = $(SRC_DIR)/client/output2all.cpp $(SRC_DIR)/client/packet.cpp $(LIB_DIR)/m-net.cpp
OUTPUT2SINGLE_SRCS = $(SRC_DIR)/client/output2single.cpp $(SRC_DIR)/client/packet.cpp $(LIB_DIR)/m-net.cpp
OUTPUT2USERS_SRCS = $(SRC_DIR)/client/output2users.cpp $(SRC_DIR)/client/packet.cpp $(LIB_DIR)/m-net.cpp
CLIENT_SRCS = $(SRC_DIR)/client/client-main.cpp $(SRC_DIR)/client/client.cpp $(SRC_DIR)/client/packet.cpp $(LIB_DIR)/m-net.cpp
SERVER_SRCS = $(SRC_DIR)/server/server-main.cpp $(SRC_DIR)/server/server.cpp $(LIB_DIR)/m-net.cpp
UI_SRCS = $(SRC_DIR)/client/start-ui.cpp

# Targets
TARGETS = $(BUILD_DIR)/input2all $(BUILD_DIR)/input2dstname $(BUILD_DIR)/input2single \
          $(BUILD_DIR)/output2all $(BUILD_DIR)/output2single $(BUILD_DIR)/output2users \
          $(BUILD_DIR)/client $(BUILD_DIR)/server \
          $(BUILD_DIR)/start-ui

# Default target
all: $(TARGETS)

# Build rules
$(BUILD_DIR)/input2all: $(INPUT2ALL_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/input2dstname: $(INPUT2DSTNAME_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/input2single: $(INPUT2SINGLE_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/output2all: $(OUTPUT2ALL_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/output2single: $(OUTPUT2SINGLE_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/output2users: $(OUTPUT2USERS_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/client: $(CLIENT_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/server: $(SERVER_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

$(BUILD_DIR)/start-ui: $(UI_SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

# Clean up generated files
clean:
	rm -f $(BUILD_DIR)/*

# PHONY targets
.PHONY: all clean
