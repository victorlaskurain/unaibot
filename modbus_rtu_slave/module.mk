local_src  := $(filter-out $(src_subdirectory)/main_custom.cpp,$(wildcard $(src_subdirectory)/*.cpp))

$(call make-library, $(subdirectory)/libmodbus_rtu_slave.a, $(local_src))
$(call make-program, $(subdirectory)/program, $(subdirectory)/main_custom.cpp, $(subdirectory)/libmodbus_rtu_slave.a timers/libtimers.a serial/libserial.a)
