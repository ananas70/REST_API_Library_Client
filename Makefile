# Set the compiler and flags
CC := g++
CFLAGS := -Wall -Wextra -std=c++17 -I./nlohmann

# Define directories
SRCDIR := src
BUILDDIR := build
TARGET := client

# Define sources and objects
C_SRCS := $(wildcard $(SRCDIR)/*.c)
CPP_SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(C_SRCS)) \
        $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(CPP_SRCS))

# Phony targets
.PHONY: all clean

# Default target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile C source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to compile C++ source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean target
clean:
	rm -rf $(BUILDDIR) $(TARGET)
