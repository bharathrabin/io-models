CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -std=gnu11

SRC_DIR   := src
BUILD_DIR := build

UNAME_S := $(shell uname -s)
LDLIBS  := -lpthread
ifeq ($(UNAME_S),Linux)
    LDLIBS += -lrt
endif

SRCS := $(wildcard $(SRC_DIR)/*.c)
BINS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%,$(SRCS))

.PHONY: all clean list
all: $(BINS)

$(BUILD_DIR)/%: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

01: $(BUILD_DIR)/01_blocking_rw
02: $(BUILD_DIR)/02_buffered_stdio
03: $(BUILD_DIR)/03_timing_harness
04: $(BUILD_DIR)/04_nonblocking
05: $(BUILD_DIR)/05_select
06: $(BUILD_DIR)/06_poll
07: $(BUILD_DIR)/07_epoll
08: $(BUILD_DIR)/08_posix_aio
09: $(BUILD_DIR)/09_threadpool_async
10: $(BUILD_DIR)/10_io_uring

list:
	@echo $(SRCS)

TESTDATA_DIR := testdata

testdata:
	mkdir -p $(TESTDATA_DIR)
	head -c 1000000    /dev/urandom > $(TESTDATA_DIR)/1mb.bin
	head -c 10000000   /dev/urandom > $(TESTDATA_DIR)/10mb.bin
	head -c 100000000  /dev/urandom > $(TESTDATA_DIR)/100mb.bin
	@echo "generated random test files in $(TESTDATA_DIR)/"
	@ls -lh $(TESTDATA_DIR)

clean-testdata:
	rm -rf $(TESTDATA_DIR)

clean:
	rm -rf $(BUILD_DIR)