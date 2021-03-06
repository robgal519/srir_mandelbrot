CC := g++
CFLAGS := -O2
UPCXX_CC := upcxx $(CFLAGS)
UPCXX_RUNNER := upcxx-run

SDL2_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL2_LFLAGS := $(shell pkg-config --libs sdl2)

BUILD_DIR := build
TARGET_DIR := target

CLIENT_EXEC := ClientMandelbrot # Also path
CLIENT_SRC := $(shell find $(CLIENT_EXEC) -name *cpp)
CLIENT_OBJS := $(CLIENT_SRC:%.cpp=$(BUILD_DIR)/%.o)
CLIENT_CFLAGS := $(CFLAGS) -I$(CLIENT_EXEC:%=%/ClientMandelbrotLib) $(SDL2_CFLAGS)
CLIENT_LFLAGS := $(SDL2_LFLAGS)

SERVER_EXEC := ServerMandelbrot # Also path
SERVER_SRC := $(shell find $(SERVER_EXEC) -name *cpp)
SERVER_OBJS := $(SERVER_SRC:%.cpp=$(BUILD_DIR)/%.o)

all: client server

run_local: all
	@rm -f /tmp/.req /tmp/.resp
	@echo Starting Client in background..
	./$(TARGET_DIR)/$(CLIENT_EXEC) &
	@echo Started Client.
	@echo Starting Server in background..
	$(UPCXX_RUNNER) -n $(shell nproc) ./$(TARGET_DIR)/$(SERVER_EXEC) &
	@echo Started Server. Nproc count $(shell nproc)

client: $(TARGET_DIR)/$(CLIENT_EXEC)

server: $(TARGET_DIR)/$(SERVER_EXEC)

clean:
	rm -rf $(BUILD_DIR) $(TARGET_DIR)

$(TARGET_DIR)/$(CLIENT_EXEC): $(CLIENT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(CLIENT_LFLAGS)

$(TARGET_DIR)/$(SERVER_EXEC): $(SERVER_OBJS)
	@mkdir -p $(dir $@)
	$(UPCXX_CC) $^ -o $@

# Variable substitution is BROKEN ! Couldn't figure out how it works.
# $(BUILD_DIR)/$(CLIENT_EXEC)/%.o:$(CLIENT_EXEC)/%.cpp
build/ClientMandelbrot/%.o:ClientMandelbrot/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CLIENT_CFLAGS) -c $< -o $@

# $(BUILD_DIR)/$(SERVER_EXEC)/%.o:$(SERVER_EXEC)/%.cpp
build/ServerMandelbrot/%.o:ServerMandelbrot/%.cpp
	@mkdir -p $(dir $@)
	$(UPCXX_CC) -c $< -o $@

.PHONY: all client server clean
