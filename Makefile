SLUG = ErraticInstruments
VERSION = 0.6.1

# Define RACK_DIR
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS +=

# Needed in order to compile on Windows (Daniele Zerbini)
# see https://www.facebook.com/groups/vcvrack/permalink/155099375150215/
include $(RACK_DIR)/arch.mk
ifeq ($(ARCH), win)
	LDFLAGS += -L$(RACK_DIR)/dep/lib -lrtmidi
endif

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*) res

# Include the VCV plugin Makefile framework
include $(RACK_DIR)/plugin.mk
