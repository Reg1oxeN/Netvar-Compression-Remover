'CC=gcc
CXX=g++

# Work around hacks in the Source engine
CFLAGS=-m32 -std=gnu++11 -fpermissive -fPIC \
	-Dstrnicmp=strncasecmp -Dstricmp=strcasecmp -D_vsnprintf=vsnprintf \
	-D_alloca=alloca -Dstrcmpi=strcasecmp -DPOSIX -DLINUX -D_LINUX -DCOMPILER_GCC

OPTFLAGS=-O2

# ******************************
# Change these to the proper
# locations for your system.
# ******************************
# The path to the Source SDK to use
HL2SDK=../hl2sdk-csgo

# Include Source SDK directories
INCLUDES=-I$(HL2SDK)/public -I$(HL2SDK)/public/tier0 -I$(HL2SDK)/public/tier1

# Include the folder with the Source SDK libraries
LINKFLAGS=-shared -m32 -L$(HL2SDK)/lib/linux

all: clean netvar_compression_remover.o netvar_compression_remover.so

netvar_compression_remover.o:
	$(CXX) $(CFLAGS) $(OPTFLAGS) $(INCLUDES) -c netvar_compression_remover.cpp

netvar_compression_remover.so:
	$(CC) -o netvar_compression_remover.so $(LINKFLAGS) netvar_compression_remover.o \
	 -l:libtier0.so -l:tier1_i486.a -lm -ldl -lstdc++

clean:
	-rm -f netvar_compression_remover.o
	-rm -f netvar_compression_remover.so
