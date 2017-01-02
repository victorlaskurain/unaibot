# source dir. Build dir expected to be a subdirectory of root
SOURCE_DIR = ..

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
STL        = /home/victor/PROIEKTUAK/arduino/libraries/avr-stl/include
TEST       = test
OBJCOPY    = avr-objcopy
BIN_FORMAT = ihex

# compiler flags
CXXFLAGS  += -Wall -g -O3 -DF_CPU=$(F_CPU) -mmcu=$(MCU) -std=c++11 -I$(STL)

# programmer configuration
AVRDUDE    = avrdude -F -V
BAUD       = 19200
PART       = m328p
PORT       = /dev/ttyACM0
PROTOCOL   = arduino

# prevent running from source directory
$(if $(filter $(notdir $(SOURCE_DIR)),$(notdir $(CURDIR))),\
  $(error Please run the makefile from the binary tree.))

# function to create static libraries
# $(call make-library, library-name, source-file-list)
define make-library
  libraries += $1
  sources   += $2
  $1: $(call source-to-object,$2)
	@$(AR) $(ARFLAGS) $$@ $$^
endef

# function to create programs
# $(call make-program, program-name, source-file-list, library-list)
define make-program
  programs  += $1.hex
  sources   += $2
  $1.hex: $1.elf
  $1.elf: $(call source-to-object,$2) $3
	$(CXX) $(CXXFLAGS) -o $$@ $$^
  .PHONY: $(subst /,_,$1)_upload
  $(subst /,_,$1)_upload: $1.hex
	$(AVRDUDE) -c $(PROTOCOL) -p $(PART) -P $(PORT) -U flash:w:$$<
endef

source-to-object = $(patsubst $(SOURCE_DIR)/%,%,$(subst .cpp,.o,$(filter %.cpp,$1)))
source-to-deps   = $(patsubst $(SOURCE_DIR)/%,%,$(subst .cpp,.d,$(filter %.cpp,$1)))

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

include_dirs := $(SOURCE_DIR)/include $(addprefix $(SOURCE_DIR)/, $(modules))
CPPFLAGS     += $(addprefix -I ,$(include_dirs))
vpath %.h   $(include_dirs)
vpath %.cpp $(SOURCE_DIR)

create-output-directories :=                            \
        $(shell for f in $(modules);                    \
                do                                      \
                  $(TEST) -d $$f || $(MKDIR) $$f;       \
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
%.d: %.cpp
	@echo $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TARGET_ARCH) -M $< | $(SED) 's,\($(notdir $*)\.o\) *:,$(dir $@)\1 $@: ,' > $@.tmp
	@$(MV) $@.tmp $@

# rule to compile c++ files to object files
%.o: %.cpp
	@echo $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# rule to generate assembly files for analysis
%.s: %.cpp
	@echo $@
	@$(CXX) $(CXXFLAGS) -S -o $@ $<

# rule to
%.hex: %.elf ; $(OBJCOPY) -O $(BIN_FORMAT) -R .eeprom $< $@

echo:
	@echo ECHO
	@echo modules: $(modules)
	@echo includes: $(patsubst %,$(SOURCE_DIR)/%/module.mk, $(modules))
	@echo sources: $(sources)
	@echo objects: $(objects)
	@echo dependencies: $(dependencies)
	@echo local_src: $(local_src)
	@echo libraries: $(libraries)
	@echo programs: $(programs)
