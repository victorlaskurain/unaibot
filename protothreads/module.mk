local_src  := $(wildcard $(src_subdirectory)/*.cpp)
include_dirs += $(SOURCE_DIR)/submodules/pt

$(call make-library, $(subdirectory)/libprotothreads.a, $(local_src))
