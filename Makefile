# Defines the compiler to be used as "gcc".
CC := gcc
# Specifies the C standard to be used, "-Wall" enables almost all compiler warnings, and "-O3" sets the optimization level to 3.
CFLAGS := -Wall -std=gnu99 -O3 -g

# Creates a list of C source files in the "src" directory 
FILES := $(wildcard src/*.c)
# Creates a list of header files in the "src" directory
HEADERS := $(wildcard src/*.h)
# Generates a list of object files by replacing the ".c" extension of each source file with ".o" and prepending "bin/" to each filename
OBJECTS := $(addprefix bin/,$(notdir $(FILES:.c=.o)))

# Sets the search path for the source files
VPATH := src

#Specifies the default target to build
all: bin ndn

#Specifies the target to build the executable named "ndn"
ndn: $(OBJECTS)
	$(CC) -o ndn $(OBJECTS)

# Specifies a pattern rule to build object files from C source files
bin/%.o: %.c | $(HEADERS)
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

# Creates the "bin" directory if it does not exist
bin:
	mkdir -p bin

# Specifies the target to clean up generated files
clean:
	rm -rf bin/* ndn