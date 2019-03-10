local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(call make-program, $(subdirectory)/program, $(local_src), motor_driver_vla/libmotor_driver_vla.a pwm/libpwm.a serial/libserial.a)
