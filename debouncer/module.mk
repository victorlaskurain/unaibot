local_src  := $(wildcard $(src_subdirectory)/src/*.cpp)

$(call make-library, $(subdirectory)/libdebouncer.a, $(local_src))
