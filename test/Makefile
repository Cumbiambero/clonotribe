# Build and run the SequencerState unit test

CXX ?= g++
CXXFLAGS ?= -std=c++23 -Wall -Wextra -I../src/dsp

all: sequencer_state_test.exe

sequencer_state_test.exe: sequencer_state_test.cpp ../src/dsp/sequencer_state.cpp
	$(CXX) $(CXXFLAGS) -I. -o $@ $^

run: sequencer_state_test.exe
	./sequencer_state_test.exe

clean:
	rm -f sequencer_state_test.exe
