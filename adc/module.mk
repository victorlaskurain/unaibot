local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(eval $(call make-library, $(subdirectory)/libadc.a, $(local_src)))