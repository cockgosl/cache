ifeq ($(origin CXX), default)
	CXX = g++
endif

CXXFLAGS ?= -g -O2 -Wall -Wextra
OUT_O_DIR ?= build
COMMONINC = -I./include
SRC = src

override CXXFLAGS += $(COMMONINC)

CXXSRC = src/main.cpp
CXXOBJ := $(addprefix $(OUT_O_DIR)/,$(CXXSRC:.cpp=.o))
DEPS = $(CXXOBJ:.o=.d)

.PHONY: all clean

all: $(OUT_O_DIR)/out

$(OUT_O_DIR)/out: $(CXXOBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(CXXOBJ): $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT_O_DIR)
