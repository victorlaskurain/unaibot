local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-library, $(subdirectory)/libpwm.a, $(local_src))
