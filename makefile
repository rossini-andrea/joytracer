CC=gcc
BUILD_DIR=build
CFLAGS=-Wall -Wextra -Wpedantic -std=c++17 -ggdb
DEPFLAGS=-MT $@ -MMD -MP -MF
LFLAGS=-lm -ldl
LIBS=stdc++ SDL2
SOURCES=\
src/joytracer.cpp \
src/sdl_wrapper.cpp \
src/sdl_main.cpp
OBJECTS=$(patsubst src/%.cpp,$(BUILD_DIR)/%.cpp.o,$(SOURCES))

.PHONY: clean all

all: joytracer

clean:
	rm -r build

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.cpp.o: src/%.cpp makefile $(BUILD_DIR)/%.cpp.d | $(BUILD_DIR)
	$(CC) $(DEPFLAGS) $(BUILD_DIR)/$*.cpp.d -c -o $@ $< $(CFLAGS)

joytracer: $(OBJECTS)
	$(CC) $(LFLAGS) -o $@ $(patsubst %,-l%,$(LIBS)) $^

DEPFILES := $(patsubst src/%.cpp,$(BUILD_DIR)/%.cpp.d,$(SOURCES))
$(DEPFILES):

include $(DEPFILES)
