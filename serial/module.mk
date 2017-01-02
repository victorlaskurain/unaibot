local_src  := $(wildcard $(src_subdirectory)/*.cpp)

$(eval $(call make-library, $(subdirectory)/libserial.a, $(local_src)))
