pin_set_clear_src := $(src_subdirectory)/pin_set_clear.cpp
pin_read_src := $(src_subdirectory)/pin_read.cpp

$(call make-program, $(subdirectory)/pin_set_clear, $(pin_set_clear_src),)
$(call make-program, $(subdirectory)/pin_read, $(pin_read_src), serial/libserial.a)
