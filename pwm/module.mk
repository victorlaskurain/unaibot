local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(eval $(call make-library, $(subdirectory)/libpwm.a, $(local_src)))
