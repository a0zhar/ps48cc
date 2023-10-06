# Compiler flags
CFLAGS = -Wall -Wno-strict-aliasing -std=gnu11 -g -I. -O0
# Object files directory
ODIR = build
# Object files generated from source files
OBJS = $(patsubst %.c, $(ODIR)/%.o, $(wildcard *.c))
# 8cc compiler executable
ECC = ./8cc
# Additional CFLAGS override to define BUILD_DIR
override CFLAGS += -DBUILD_DIR='"$(shell pwd)"'

# Build the 8cc compiler
8cc: 8cc.h $(OBJS)
	@echo "Compiling 8cc..."
	@cc -o $@ $(OBJS) $(LDFLAGS)
	@echo "8CC Compiled Successfully"

# Compile individual source files into object files
$(ODIR)/%.o: %.c | $(ODIR)
	@$(CC) $(CFLAGS) -o $@ -c $<
	@echo "Successfully compiled $<"

# Create build directory if it doesn't exist
$(ODIR):
	@mkdir -p $(ODIR)

# Dependencies for object files
$(OBJS): 8cc.h keyword.inc

# Clean the project
clean:
	@echo "Cleaning Up"
	@rm -f 8cc
	@rm -rf $(ODIR)
	@echo "Cleaned!"

# Default target: build the 8cc compiler
all: 8cc

.PHONY: clean all
