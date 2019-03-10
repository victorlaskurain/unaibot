local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-program, $(subdirectory)/program, $(local_src),)
