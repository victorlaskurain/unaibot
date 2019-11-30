local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-library,$(subdirectory)/proto_plant_main.a,$(local_src))

$(call make-program,$(subdirectory)/program,,modbus_rtu_slave/libmodbus_rtu_slave.a $(subdirectory)/proto_plant_main.a timers/libtimers.a serial/libserial.a)
