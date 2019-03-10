local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-library, $(subdirectory)/libtimers.a, $(local_src))
