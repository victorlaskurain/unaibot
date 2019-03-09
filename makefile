# source dir.
SOURCE_DIR := $(realpath $(dir $(realpath $(firstword $(MAKEFILE_LIST)))))
BUILD_DIR  := $(if $(BUILD_DIR),$(BUILD_DIR),$(SOURCE_DIR)/build)
make-build-dir := $(shell mkdir -p $(BUILD_DIR))

# device settings
F_CPU      = 16000000UL
MCU        = atmega328p

# commands
AR         = avr-ar
CC         = avr-gcc
CXX        = avr-g++
MKDIR      = mkdir -p
MV         = mv -f
RM         = rm -f
SED        = sed
TEST       = test
OBJCOPY    = avr-objcopy
BIN_FORMAT = ihex

# compiler flags
CXXFLAGS  += -Wall -g -O3 -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=c++11

# programmer configuration
AVRDUDE    = avrdude -F -V
DEVICE     = device_not_specified
include $(SOURCE_DIR)/device_conf/$(DEVICE).conf

# verbosity control
Q          = $(if $(VERBOSE),,@)

# Function to create static libraries
# $(call make-library, library-name, source-file-list)
define make-library
  libraries += $(addprefix $(BUILD_DIR)/,$1)
  sources   += $2
  $(addprefix $(BUILD_DIR)/,$1): $(call source-to-object,$2)
	@echo Build library $$@
	$Q$(AR) $(ARFLAGS) $$@ $$^ >/dev/null
endef

# Function to create programs.
#
# Also creates a phony target to upload the program to the arduino. If
# the program is serial_test/program, then the upload target will be
# serial_test_program_upload.
#
# $(call make-program, program-name, source-file-list, library-list)
define make-program
  programs  += $(addprefix $(BUILD_DIR)/,$1.hex)
  sources   += $2
  $(addprefix $(BUILD_DIR)/,$1.hex): $(addprefix $(BUILD_DIR)/,$1.elf)
  $(addprefix $(BUILD_DIR)/,$1.elf): $(call source-to-object,$2) $(addprefix $(BUILD_DIR)/,$3)
	@echo Build program $$@
	$Q$(CXX) $(CXXFLAGS) -o $$@ $$^
  .PHONY: $(subst /,_,$1)_upload
  $(subst /,_,$1)_upload: $(addprefix $(BUILD_DIR)/,$1.hex)
	@echo upload
	$(AVRDUDE) -b $(BAUD) -c $(PROTOCOL) -p $(PART) -P $(PORT) -U flash:w:$$<
endef

source-to-object = $(patsubst $(SOURCE_DIR)/%,$(BUILD_DIR)/%,$(subst .cpp,.o,$(filter %.cpp,$1)))
source-to-deps   = $(patsubst $(SOURCE_DIR)/%,$(BUILD_DIR)/%,$(subst .cpp,.d,$(filter %.cpp,$1)))

subdirectory = $(patsubst $(SOURCE_DIR)/%/module.mk,%,  \
                 $(word                               \
                   $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
src_subdirectory = $(patsubst %/module.mk,%,  \
                 $(word                               \
                   $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

# Collect information from each module in these four variables.
# Initialize them here as simple variables.
modules      := $(patsubst $(SOURCE_DIR)/%/module.mk,%,$(shell find $(SOURCE_DIR) -name module.mk))
programs     :=
sources      :=
libraries    :=
sources      :=

objects      = $(call source-to-object,$(sources))
dependencies = $(call source-to-deps,$(sources))

# add all modules to the include dir, set search paths for source files
include_dirs := $(SOURCE_DIR)/include $(addprefix $(SOURCE_DIR)/, $(addsuffix /include,$(modules)))
CPPFLAGS     += $(addprefix -I ,$(include_dirs))
vpath %.h   $(include_dirs)
vpath %.hpp $(include_dirs)
vpath %.cpp $(SOURCE_DIR)

# create output directories as soon as possible
create-output-directories :=										\
        $(shell for f in $(modules);								\
                do													\
                  f=$(BUILD_DIR)/$$f;                               \
                  $(TEST) -d $$f         || $(MKDIR) $$f;			\
                  $(TEST) -d $$f/include || $(MKDIR) $$f/include;   \
                done)

all:

include $(patsubst %,$(SOURCE_DIR)/%/module.mk, $(modules))

.PHONY: all
all: libraries $(programs)

.PHONY: libraries
libraries: $(libraries)

.PHONY: clean
clean:
	$(RM) $(objects) $(programs) $(libraries) $(dependencies) $(extra_clean)

ifneq "$(MAKECMDGOALS)" "clean"
  -include $(dependencies)
endif

# rule to create dependency files
$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.cpp
	@echo Compute dependencies of $(subst $(SOURCE_DIR)/,,$<)
	$Q$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TARGET_ARCH) -M $< | $(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp
	$Q$(MV) $@.tmp $@

# rule to compile c++ files to object files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@echo Compile $(subst $(SOURCE_DIR)/,,$<)
	$Q$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# rule to generate assembly files for analysis
%.s: %.cpp
	@echo Create assembly file for $(subst $(SOURCE_DIR)/,,$<)
	$Q$(CXX) $(CPPFLAGS) $(CXXFLAGS) -S -o $@ $<

# rule to generate the hex file (ready to upload) from the elf binary
%.hex: %.elf
	@echo $(subst $(SOURCE_DIR)/,,$<)
	$Q$(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom $< $@
