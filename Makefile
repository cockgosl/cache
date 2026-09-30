ifeq ($(origin CXX), default)
	CXX = g++
endif

CXXFLAGS ?= -g -O2 -fsanitize=address -Wall -Wextra -std=c++17
OUT_O_DIR ?= build
COMMONINC = -I./include
SRC = src
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

override CXXFLAGS += $(COMMONINC)

CXXSRC = src/main.cpp src/cache_api.cpp

CXXOBJ := $(addprefix $(OUT_O_DIR)/,$(CXXSRC:.cpp=.o))

DEPS = $(CXXOBJ:.o=.d)

.PHONY: all
all: $(OUT_O_DIR)/out

$(OUT_O_DIR)/out : $(CXXOBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(CXXOBJ) : $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	$(CXX) -E $(CXXFLAGS) $< -MM -MT $(@:.d=.o) > $@

-include $(DEPS)

# Tests
TEST_DIR = tests
TEST_BIN = build/cache_tests

TEST_SRC = $(wildcard $(TEST_DIR)/*_tests.cpp)

GTEST_LIBS = -lgtest -lgtest_main -pthread

.PHONY: test test-filter clean

test: $(TEST_BIN)
	./$(TEST_BIN)

test-filter: $(TEST_BIN)
	./$(TEST_BIN) --gtest_filter="$(FILTER)"

$(TEST_BIN): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(COMMONINC) $(TEST_SRC) $(GTEST_LIBS) -o $@

clean:
	rm -rf $(OUT_O_DIR)
