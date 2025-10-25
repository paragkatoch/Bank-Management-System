# # Compiler and flags
# CC = gcc
# CFLAGS = -Wall -Iinclude -g

# # Directories
# SRC_DIR = src
# NODE_DIR = node
# BUILD_DIR = build

# # Executable names
# SERVER_EXE = $(BUILD_DIR)/server
# CLIENT_EXE = $(BUILD_DIR)/client

# # Source files
# COMMON_SRCS = $(wildcard $(SRC_DIR)/*.c)
# SERVER_SRC = $(NODE_DIR)/server.c $(COMMON_SRCS)
# CLIENT_SRC = $(NODE_DIR)/client.c $(COMMON_SRCS)

# # Object files
# SERVER_OBJS = $(SERVER_SRC:.c=$(BUILD_DIR)/.o)
# CLIENT_OBJS = $(CLIENT_SRC:.c=$(BUILD_DIR)/.o)

# # Default target
# all: $(SERVER_EXE) $(CLIENT_EXE)

# # Build server
# $(SERVER_EXE):
# 	@mkdir -p $(BUILD_DIR)
# 	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_EXE)

# # Build client
# $(CLIENT_EXE):
# 	@mkdir -p $(BUILD_DIR)
# 	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_EXE)

# # Clean
# clean:
# 	rm -rf $(BUILD_DIR)


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

# Source files
COMMON_SRCS = $(wildcard $(SRC_DIR)/*.c)
SERVER_SRC = $(NODE_DIR)/server.c $(COMMON_SRCS)
CLIENT_SRC = $(NODE_DIR)/client.c $(COMMON_SRCS)

# Default target
all: server client

# Force rebuild every time
.PHONY: all server client clean

server:
	@mkdir -p $(BUILD_DIR)
	@echo "🔧 Building server..."
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_EXE)

client:
	@mkdir -p $(BUILD_DIR)
	@echo "💻 Building client..."
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_EXE)

clean:
	@echo "🧹 Cleaning build directory..."
	rm -rf $(BUILD_DIR)
