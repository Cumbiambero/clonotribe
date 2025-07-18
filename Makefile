RACK_DIR ?= $(RACK_DIR)
SLUG = ClonoTribe
VERSION = 2.0.0

FLAGS += -isystem $(RACK_DIR)/include
FLAGS += -std=c++23 -Wall -Wextra -Wpedantic -Wconversion -Wno-psabi

SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/ui/*.cpp)
SOURCES += $(wildcard src/dsp/*.cpp)

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

include $(RACK_DIR)/plugin.mk
