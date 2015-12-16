CXXFLAGS=-std=c++14

LDLIBS+=-lboost_system
LDLIBS+=-lboost_filesystem
LDLIBS+=-lstdc++


.PHONY: default-target
default-target: linux_hotkey

%: %.cpp

linux_hotkey.o: virtual_keyboard.h

.PHONY: clean
clean:
	$(RM) linux_hotkey *.o
