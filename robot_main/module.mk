msv1_src := $(wildcard $(src_subdirectory)/main_msv1.cpp)
mvla_src := $(wildcard $(src_subdirectory)/main_mvla.cpp)

$(eval $(call make-program, $(subdirectory)/program_msv1, $(msv1_src), command/libcommand.a motor_driver/libmotor_driver.a pwm/libpwm.a shift_register/libshift_register.a serial/libserial.a))
$(eval $(call make-program, $(subdirectory)/program_mvla, $(mvla_src), command/libcommand.a motor_driver/libmotor_driver.a pwm/libpwm.a shift_register/libshift_register.a serial/libserial.a))
