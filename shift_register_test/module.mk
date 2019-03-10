local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-program, $(subdirectory)/program, $(local_src), shift_register/libshift_register.a serial/libserial.a)
