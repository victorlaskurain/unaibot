local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(eval $(call make-program, $(subdirectory)/program, $(local_src), motor_driver/libmotor_driver.a pwm/libpwm.a shift_register/libshift_register.a serial/libserial.a))
