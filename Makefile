ifeq ($(origin CXX), default) 
	CXX = g++
endif

CXXFLAGS ?= -g -no-pie -O2 -Wall -Wextra
OUT_O_DIR ?= build
COMMONINC = -I./include
SRC = src
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

override CXXFLAGS += $(COMMONINC)

CXXSRC = src/main.cpp

CXXOBJ := $(addprefix $(OUT_O_DIR)/,$(CXXSRC:.cpp=.o)) 

DEPS = $(CXXOBJ:.o=.d)

.PHONY: all
all: $(OUT_O_DIR)/out

$(OUT_O_DIR)/out : $(CXXOBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(CXXOBJ) : $(OUT_O_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(DEPS) : $(OUT_O_DIR)/%.d : %.cpp
	@mkdir -p $(@D)
	$(CXX) -E $(CXXFLAGS) $< -MM -MT $(@:.d=.o) > $@


PHONY: clean
clean:
	rm -rf $(OUT_O_DIR)
