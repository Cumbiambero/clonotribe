RACK_DIR ?= ../..

FLAGS += -isystem
FLAGS += -Wall -Wextra -Wpedantic -Wconversion -Wno-psabi
CXXFLAGS += -std=c++23

SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/ui/*.cpp)
SOURCES += $(wildcard src/dsp/*.cpp)

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

include $(RACK_DIR)/plugin.mk

CXXFLAGS := $(filter-out -std=c++11,$(CXXFLAGS))
