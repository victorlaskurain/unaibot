local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(eval $(call make-program, $(subdirectory)/program, $(local_src), adc/libadc.a serial/libserial.a))
