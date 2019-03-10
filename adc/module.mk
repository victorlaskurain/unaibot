local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-library, $(subdirectory)/libadc.a, $(local_src))
