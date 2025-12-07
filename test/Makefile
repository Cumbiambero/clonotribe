CXX ?= g++
EXEEXT ?= .exe
RM ?= rm -f

CXXFLAGS ?= -std=c++23 -O2 -Wall -I../src
FLAGS += -Wpedantic -Wconversion -Wno-psabi

# Only compile the main test file, not analysis/comparison tools
SOURCES = ms20.cpp
BUILDDIR = build
TARGET = $(BUILDDIR)/test$(EXEEXT)

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(SOURCES) doctest.h | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(FLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
