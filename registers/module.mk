local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-library, $(subdirectory)/libshift_register.a, $(local_src))
