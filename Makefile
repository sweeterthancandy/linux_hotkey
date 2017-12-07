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

ALLHEADERS= event_monitor.h keyboard_state.h key_conv.h pattern_matcher.h virtual_keyboard.h
linux_hotkey.o: $(ALLHEADERS)
virtual_keyboard.o:$(ALLHEADERS) 

.PHONY: clean
clean:
	$(RM) linux_hotkey *.o
