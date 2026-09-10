EE_BIN = ../bin/PS2-NativeHooks.elf
EE_OBJS = main.o
EE_INCS = -I.
EE_LIBS = -ldebug -lkernel

all: $(EE_BIN)

clean:
	rm -f $(EE_OBJS) $(EE_BIN)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
