# Compiler and flags
CC = gcc
CFLAGS = -Wall -Iinclude -g

# Directories
SRC_DIR = src
NODE_DIR = node
BUILD_DIR = build

# Executable names
SERVER_EXE = $(BUILD_DIR)/server
CLIENT_EXE = $(BUILD_DIR)/client

# Source files (recursive)
COMMON_SRCS = $(shell find $(SRC_DIR) -name '*.c')
SERVER_SRC = $(NODE_DIR)/server.c $(COMMON_SRCS)
CLIENT_SRC = $(NODE_DIR)/client.c $(COMMON_SRCS)

# Default target
all: build_server build_client

.PHONY: all server client clean build_server build_client

build_server:
	@mkdir -p $(BUILD_DIR)
	@echo "\nBuilding server...\n"
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_EXE)

build_client:
	@mkdir -p $(BUILD_DIR)
	@echo "\nBuilding client...\n"
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_EXE)

clean:
	@echo "\nCleaning build directory...\n"
	rm -rf $(BUILD_DIR)

server:
	./build/server

client:
	./build/client
