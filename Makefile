CXX=g++
CXXFLAGS=-std=c++11 -g -pthread

CC_SRCS=\
	main.cpp\
	msr.cpp\
	mem_lines.cpp\
	pmon.cpp\
	tasks.cpp\
	cpuid_tasks.cpp\

TARGET=core_map

OBJS_DIR := objs
SRCS_DIR := srcs

VPATH=$(SRCS_DIR)

CC_SRCS_ := $(patsubst %.cpp,$(SRCS_DIR)/%.cpp,$(CC_SRCS))
OBJS := $(patsubst %.cpp,$(OBJS_DIR)/%.o,$(CC_SRCS))
DEPS := $(patsubst %.cpp,$(OBJS_DIR)/%.d,$(CC_SRCS))

all: $(TARGET)

-include $(DEPS)

$(OBJS_DIR)/%.o:%.cpp
	mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) $< -c -MMD -MP -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

.PHONY=clean

clean:
	rm -f $(OBJS) $(TARGET) $(DEPS)

