CC = gcc
CFLAGS = -g -Iinclude -I../utility/include
LDFLAGS = -lpthread -lm
SRC_DIR = src
UTILITY_SRC_DIR = ../utility/src
BIN_DIR = bin
BUILD_DIR = build
TEST_DIR = test

SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(UTILITY_SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c)) \
       $(patsubst $(UTILITY_SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(wildcard $(UTILITY_SRC_DIR)/*.c))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c, $(BIN_DIR)/%, $(TEST_SRCS))

TARGET = mission_supervisor

all: $(TARGET)

$(TARGET): $(OBJS) $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(UTILITY_SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

NO_MAIN_OBJS := $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

$(BIN_DIR)/%: $(TEST_DIR)/%.c | $(BIN_DIR) $(TARGET)
	$(CC) $(CFLAGS) $< -o $@ $(NO_MAIN_OBJS) $(LDFLAGS)

.PHONY: test
test: $(TEST_BINS)
	@echo "----------------\nRunning tests...\n----------------"
	@for t in $(TEST_BINS); do \
	  echo "Running $$t..."; \
	  $$t || exit 1; \
	done

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)