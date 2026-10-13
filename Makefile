ifeq ($(origin CXX), default)
    CXX = g++
endif

CXXFLAGS ?= -g -O2 -fsanitize=address -Wall -Wextra -std=c++17

OUT_O_DIR ?= build
COMMONINC = -I./include

override CXXFLAGS += $(COMMONINC)

# Main application
CXXSRC = src/main.cpp src/cache_api.cpp
CXXOBJ := $(addprefix $(OUT_O_DIR)/,$(CXXSRC:.cpp=.o))
DEPS := $(CXXOBJ:.o=.d)

# Tests
TEST_DIR = tests
TEST_BIN = $(OUT_O_DIR)/cache_tests
TEST_SRC = $(wildcard $(TEST_DIR)/*_tests.cpp)
TEST_OBJ := $(addprefix $(OUT_O_DIR)/,$(TEST_SRC:.cpp=.o))
TEST_DEPS := $(TEST_OBJ:.o=.d)
GTEST_LIBS = -lgtest -lgtest_main -pthread

# Benchmarks
BENCH_SRC = benchmarks/benchmark.cpp
BENCH_OUT = $(OUT_O_DIR)/benchmark
BENCH_OBJ = $(OUT_O_DIR)/benchmarks/benchmark.o
BENCH_DEPS = $(BENCH_OBJ:.o=.d)

.PHONY: all test test-filter benchmark benchmarks clean

all: $(OUT_O_DIR)/out

# Main application
$(OUT_O_DIR)/out: $(CXXOBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile source files and generate dependencies
$(OUT_O_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Tests
test: $(TEST_BIN)
	./$(TEST_BIN)

test-filter: $(TEST_BIN)
	./$(TEST_BIN) --gtest_filter="$(FILTER)"

$(TEST_BIN): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ $(GTEST_LIBS) -o $@ $(LDFLAGS)

# Benchmarks
benchmark: $(BENCH_OUT)

benchmarks: $(BENCH_OUT)
	./$(BENCH_OUT)
	@echo "Generating plots..."
	python3 benchmarks/plot_benchmarks.py

$(BENCH_OUT): $(BENCH_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Include generated dependencies
-include $(DEPS) $(TEST_DEPS) $(BENCH_DEPS)

clean:
	rm -rf $(OUT_O_DIR)
	rm -rf benchmarks/plots
	rm -f benchmarks/benchmarks.csv
