RACK_DIR ?= $(RACK_DIR)
SLUG = ClonoTribe
VERSION = 2.0.0

FLAGS += -Idep/include

SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/ui/*.cpp)
SOURCES += $(wildcard src/dsp/*.cpp)

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

include $(RACK_DIR)/plugin.mk
