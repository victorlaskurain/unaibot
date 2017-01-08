local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(eval $(call make-library, $(subdirectory)/libmotor_driver.a, $(local_src)))
