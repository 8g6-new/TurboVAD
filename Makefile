CC = gcc

# beyond O2 it's found that NAN == NAN (UB)

#_______________________General optimization flags__________________________________
OPTFLAGS = -Ofast                   # Enables aggressive optimizations beyond -O2, including unsafe floating-point optimizations
OPTFLAGS += -march=native           # Generates code optimized for the host CPU
OPTFLAGS += -mtune=native           # Optimizes code scheduling for the host CPU
OPTFLAGS += -funroll-loops          # Unrolls loops to reduce branch overhead
OPTFLAGS += -fpeel-loops            # Extracts loop iterations that always execute to optimize performance
OPTFLAGS += -ftracer                # Improves branch prediction and inlining
OPTFLAGS += -flto                   # Enables Link-Time Optimization (LTO) for cross-file optimization
OPTFLAGS += -fuse-linker-plugin     # Uses a linker plugin for better LTO performance
OPTFLAGS += -MMD -MP                # Generates dependency files for make without including system headers
OPTFLAGS += -floop-block            # Optimizes loop memory access patterns for better cache performance
OPTFLAGS += -floop-interchange      # Reorders nested loops for better vectorization and locality
OPTFLAGS += -floop-unroll-and-jam   # Unrolls outer loops and fuses iterations for better performance
OPTFLAGS += -fipa-pta               # Enables interprocedural pointer analysis for optimization
OPTFLAGS += -fipa-cp                # Performs constant propagation across functions
OPTFLAGS += -fipa-sra               # Optimizes function arguments and return values for efficiency
OPTFLAGS += -fipa-icf               # Merges identical functions to reduce code size

#_______________________Says compiler to vectorize loops_________________________________________
VECFLAGS = -ftree-vectorize         # Enables automatic vectorization of loops
VECFLAGS += -ftree-loop-vectorize   # Enables loop-based vectorization
VECFLAGS += -fopt-info-vec-optimized # Outputs details of vectorized loops

#_______________________SIMD Instructions that are used for vectorizing _________________________________________
VECFLAGS += -mavx                   # SIMD flags
VECFLAGS += -mavx2                  # SIMD flags
VECFLAGS += -mfma                   # Enables Fused Multiply-Add (FMA) instructions
VECFLAGS += -msse4.2                # Enables SSE4.2 instruction set
VECFLAGS += -mabm                   # Enables Advanced Bit Manipulation instructions
VECFLAGS += -mf16c                  # Enables 16-bit floating-point conversion instructions

#_______________________Debugging and safety flags__________________________________
DBGFLAGS = -Og                      # Optimizations suitable for debugging
DBGFLAGS += -fno-omit-frame-pointer # Keeps the frame pointer for debugging
DBGFLAGS += -fno-inline             # Disables function inlining for better debugging
DBGFLAGS += -fstack-protector-strong # Adds stack protection to detect buffer overflows
DBGFLAGS += -g                      # Generates debugging information
DBGFLAGS += -fsanitize=address      # Enables AddressSanitizer for memory error detection
DBGFLAGS += -fsanitize=leak         # Enables leak detection
DBGFLAGS += -fsanitize=undefined    # Enables Undefined Behavior Sanitizer (UBSan)

LIBFLAGS = -DMINIMP3_FLOAT_OUTPUT -fopenmp 

WARNFLAGS = -Wall -Wextra 

CFLAGS = $(WARNFLAGS) $(OPTFLAGS) $(VECFLAGS) $(LIBFLAGS)
LDFLAGS = -lm -lfftw3 -lfftw3f -lsndfile -lpng -lopenblas 

# Directory structure
SRCDIR = src
SCHEMEDIR = $(SRCDIR)/colorschemes
BUILDDIR = build

# Output directories
OUT_DIRS = ./out/stft/bird ./out/stft/nobird \
           ./out/mel/bird ./out/mel/nobird \
           ./out/wav/bird ./out/wav/nobird

# Create build directory structure
$(shell mkdir -p $(BUILDDIR))
$(shell mkdir -p $(BUILDDIR)/$(SRCDIR)/libheatmap)
$(shell mkdir -p $(BUILDDIR)/$(SRCDIR)/png_tools)
$(shell mkdir -p $(BUILDDIR)/$(SRCDIR)/utils)
$(shell mkdir -p $(BUILDDIR)/$(SRCDIR)/ml)
$(shell mkdir -p $(BUILDDIR)/$(SRCDIR)/audio_tools)
$(shell mkdir -p $(BUILDDIR)/$(SCHEMEDIR)/builtin)
$(shell mkdir -p $(BUILDDIR)/$(SCHEMEDIR)/opencv_like)

# Source files
BASE_SOURCES = mean.c \
               $(wildcard $(SRCDIR)/libheatmap/*.c) \
               $(wildcard $(SRCDIR)/png_tools/*.c) \
               $(wildcard $(SRCDIR)/utils/*.c) \
               $(wildcard $(SRCDIR)/ml/*.c) \
               $(wildcard $(SRCDIR)/audio_tools/*.c)

# Color scheme sources
BUILTIN_DIR = $(SCHEMEDIR)/builtin
OPENCV_DIR = $(SCHEMEDIR)/opencv_like
BUILTIN_SOURCES = $(wildcard $(BUILTIN_DIR)/*.c)
OPENCV_SOURCES = $(wildcard $(OPENCV_DIR)/*.c)

# Object files
BASE_OBJECTS = $(patsubst %.c,$(BUILDDIR)/%.o,$(BASE_SOURCES))
BUILTIN_OBJECTS = $(patsubst $(BUILTIN_DIR)/%.c,$(BUILDDIR)/$(BUILTIN_DIR)/%.o,$(BUILTIN_SOURCES))
OPENCV_OBJECTS = $(patsubst $(OPENCV_DIR)/%.c,$(BUILDDIR)/$(OPENCV_DIR)/%.o,$(OPENCV_SOURCES))

# Dependency files
DEPS = $(BASE_OBJECTS:.o=.d) $(BUILTIN_OBJECTS:.o=.d) $(OPENCV_OBJECTS:.o=.d)

# Track last built target
LAST_TARGET_FILE = .last_target
ifneq ($(wildcard $(LAST_TARGET_FILE)),)
    LAST_TARGET := $(shell cat $(LAST_TARGET_FILE))
else
    LAST_TARGET := builtin
endif

# Parameters for running the program
FOL = "/home/dsb/disks/data/paper/c/c_spectrogram/tests/files/Voice of Birds/ana/det/audio/25/Blue Jay/"
FILE = "0.mp3"
PARAMS = ${FOL}${FILE} 512 2048 128 128 "hann" "hann" 0.5 256 0 7500 \
         "./out/wav" "./out/stft" "./out/mel" "./cache/FFT" 89 89 0.01

# Phony targets
.PHONY: all clean debug test opencv_like builtin run run_debug shared clean_fols create_dirs

# Default target
all: $(LAST_TARGET)

# Create output directories
create_dirs:
	mkdir -p $(OUT_DIRS)
	mkdir -p ./cache/FFT

# OpenCV Color Scheme Build
opencv_like: create_dirs
	$(MAKE) _opencv_like

_opencv_like: CFLAGS += -DOPENCV_LIKE
_opencv_like: $(BASE_OBJECTS) $(OPENCV_OBJECTS)
	$(CC) $(CFLAGS) -o opencv_like $^ $(LDFLAGS)
	@echo "opencv_like" > $(LAST_TARGET_FILE)
	@echo "Built with OpenCV-like color scheme"

# Builtin Color Scheme Build
builtin: create_dirs
	$(MAKE) _builtin

_builtin: CFLAGS += -DBUILTIN
_builtin: $(BASE_OBJECTS) $(BUILTIN_OBJECTS)
	$(CC) $(CFLAGS) -o builtin $^ $(LDFLAGS)
	@echo "builtin" > $(LAST_TARGET_FILE)
	@echo "Built with Builtin color scheme"

# Shared Library Build
shared: $(BASE_OBJECTS) $(BUILTIN_OBJECTS) $(OPENCV_OBJECTS)
	$(CC) -shared -o libyourlib.so $^ $(LDFLAGS)
	@echo "shared" > $(LAST_TARGET_FILE)
	@echo "Shared library libyourlib.so built successfully."

# Compilation Rule with Dependency Tracking for main file
$(BUILDDIR)/mean.o: mean.c
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP

# Compilation Rule with Dependency Tracking for source files in subdirectories
$(BUILDDIR)/$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MP

# Special rules for colorscheme directories
$(BUILDDIR)/$(BUILTIN_DIR)/%.o: $(BUILTIN_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DBUILTIN -c $< -o $@ -MD -MP

$(BUILDDIR)/$(OPENCV_DIR)/%.o: $(OPENCV_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DOPENCV_LIKE -c $< -o $@ -MD -MP

# Include generated dependency files
-include $(DEPS)

# Debug Build
debug: CFLAGS = $(WARNFLAGS) $(DBGFLAGS) $(LIBFLAGS)
debug: clean
	$(MAKE) $(LAST_TARGET)
	@echo "Built in Debug Mode"

# Run with valgrind
run_debug:
	@if [ ! -f "$(LAST_TARGET_FILE)" ] || [ ! -x "$$(cat $(LAST_TARGET_FILE))" ]; then \
		echo "No valid executable found. Run 'make' first."; exit 1; \
	fi; \
	LAST_TARGET=$$(cat $(LAST_TARGET_FILE)); \
	if [ "$$LAST_TARGET" = "shared" ]; then \
		echo "Last build was a shared library. Nothing to run."; exit 1; \
	fi; \
	echo "Running $$LAST_TARGET with Valgrind..."; \
	valgrind --leak-check=full --show-leak-kinds=definite,possible ./$$LAST_TARGET $(PARAMS)

# Run program normally
run:
	@if [ ! -f "$(LAST_TARGET_FILE)" ] || [ ! -x "$$(cat $(LAST_TARGET_FILE))" ]; then \
		echo "No valid executable found. Run 'make' first."; exit 1; \
	fi; \
	LAST_TARGET=$$(cat $(LAST_TARGET_FILE)); \
	if [ "$$LAST_TARGET" = "shared" ]; then \
		echo "Last build was a shared library. Nothing to run."; exit 1; \
	fi; \
	echo "Running $$LAST_TARGET..."; \
	./$$LAST_TARGET $(PARAMS)

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)
	rm -f builtin opencv_like libyourlib.so $(LAST_TARGET_FILE)

# Clean output folders
clean_fols:
	$(foreach dir, $(OUT_DIRS), rm -f $(dir)/*.png $(dir)/*.wav;)