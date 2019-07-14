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
CXXFLAGS  += -Wall -O3 -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=c++11
# see http://kibergus.su/en/node/92
CXXFLAGS  += -fno-threadsafe-statics

# programmer configuration
AVRDUDE    = avrdude -F -V
DEVICE     := $(if $(DEVICE),$(DEVICE),uno)
include $(SOURCE_DIR)/device_conf/$(DEVICE).conf

# verbosity control
Q          = $(if $(VERBOSE),,@)

# Function to create static libraries
# $(call make-library, library-name, source-file-list)
define make-library-rule
  libraries += $(addprefix $(BUILD_DIR)/,$1)
  sources   += $2
  $(call source-to-object,$2): $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
  $(addprefix $(BUILD_DIR)/,$1): $(call source-to-object,$2)
	$$(info Build library $$@)
	$Q$(AR) $(ARFLAGS) $$@ $$^ >/dev/null
endef
make-library = $(eval $(call make-library-rule,$1,$2,$3))


# Function to create programs.
#
# Also creates a phony target to upload the program to the arduino. If
# the program is serial_test/program, then the upload target will be
# serial_test_program_upload.
#
# $(call make-program, program-name, source-file-list, library-list)
define make-program-rule
  programs      += $(addprefix $(BUILD_DIR)/,$1.hex)
  sources       += $2
  other_targets += $(subst /,_,$1)_upload
  $(addprefix $(BUILD_DIR)/,$1.hex): $(addprefix $(BUILD_DIR)/,$1.elf)
  $(addprefix $(BUILD_DIR)/,$1.elf): $(call source-to-object,$2) $(addprefix $(BUILD_DIR)/,$3)
	$$(info Build program $$@)
	$Q$(CXX) $(CXXFLAGS) -o $$@ $$^
  .PHONY: $(subst /,_,$1)_upload
  $(subst /,_,$1)_upload: $(addprefix $(BUILD_DIR)/,$1.hex)
	$$(info upload)
	$(AVRDUDE) -b $(BAUD) -c $(PROTOCOL) -p $(PART) -P $(PORT) -U flash:w:$$<
endef
make-program = $(eval $(call make-program-rule,$1,$2,$3))

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
modules       := $(patsubst $(SOURCE_DIR)/%/module.mk,%,$(shell find $(SOURCE_DIR) -name module.mk))
programs      :=
sources       :=
libraries     :=
sources       :=
other_targets :=

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

ifeq "$(MAKECMDGOALS)" "clean"
else ifeq "$(MAKECMDGOALS)" "help"
else
  -include $(dependencies)
endif

# rule to create dependency files
$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.cpp
	$(info Compute dependencies of $(subst $(SOURCE_DIR)/,,$<))
	$Q$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TARGET_ARCH) -M $< | $(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp
	$Q$(MV) $@.tmp $@

# rule to compile c++ files to object files
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(SOURCE_DIR)/makefile
	$(info Compile $(subst $(SOURCE_DIR)/,,$<))
	$Q$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# rule to generate assembly files for analysis
%.s: %.cpp
	$(info Create assembly file for $(subst $(SOURCE_DIR)/,,$<))
	$Q$(CXX) $(CPPFLAGS) $(CXXFLAGS) -S -o $@ $<

# rule to generate the hex file (ready to upload) from the elf binary
%.hex: %.elf
	$(info Generate hex file $@)
	$Q$(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom $< $@

# show usage
define n


endef
blank :=
s := $(blank) $(blank)
.PHONY: help
help:
	$(info Usage:)
	$(info make BUILD_DIR=<build dir> DEVICE=<device>)
	$(info $s$s- BUILD_DIR: where to put the generated files. Default build.)
	$(info $s$s- DEVICE: device configuration. Choose one of [$(basename $(notdir $(wildcard $(SOURCE_DIR)/device_conf/*)))]. Default uno.)
	$(info Source dir: $n$s$s- $(SOURCE_DIR))
	$(info Build dir: $n$s$s- $(BUILD_DIR))
	$(info Library targets:    $(patsubst $(BUILD_DIR)/%,$n  - $$BUILD_DIR/%,$(sort $(libraries))))
	$(info Executable targets: $(patsubst $(BUILD_DIR)/%,$n  - $$BUILD_DIR/%,$(sort $(programs))))
	$(info Other targets: $(patsubst %,$n  - %,$(sort $(other_targets))))
