CXXFLAGS+=-std=c++14
CXXFLAGS+=-ggdb3
CPPFLAGS+=-DBOOST_LOG_DYN_LINK

LDLIBS+=-lboost_system
LDLIBS+=-lboost_filesystem
LDLIBS+=-lboost_program_options
LDLIBS+=-lboost_log
LDLIBS+=-lstdc++
LDLIBS+=-lpthread


.PHONY: default-target
default-target: linux_hotkey

linux_hotkey: linux_hotkey.o virtual_keyboard.o

%: %.cpp

linux_hotkey.o: virtual_keyboard.h 
virtual_keyboard.o: virtual_keyboard.h

.PHONY: clean
clean:
	$(RM) linux_hotkey *.o
