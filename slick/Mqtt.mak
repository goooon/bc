# SlickEdit generated file.  Do not edit this file except in designated areas.

# Make command to use for dependencies
MAKE=make
RM=rm
MKDIR=mkdir

# -----Begin user-editable area-----

# -----End user-editable area-----

# If no configuration is specified, "Debug" will be used
ifndef CFG
CFG=Debug
endif

#
# Configuration: Debug
#
ifeq "$(CFG)" "Debug"
OUTDIR=Debug
OUTFILE=$(OUTDIR)/Mqtt.so
CFG_INC=
CFG_LIB=
CFG_OBJ=
COMMON_OBJ=$(OUTDIR)/Clients.o $(OUTDIR)/Heap.o \
	$(OUTDIR)/LinkedList.o $(OUTDIR)/Log.o $(OUTDIR)/Messages.o \
	$(OUTDIR)/MQTTAsync.o $(OUTDIR)/MQTTPacket.o \
	$(OUTDIR)/MQTTPacketOut.o $(OUTDIR)/MQTTPersistence.o \
	$(OUTDIR)/MQTTPersistenceDefault.o $(OUTDIR)/MQTTProtocolClient.o \
	$(OUTDIR)/MQTTProtocolOut.o $(OUTDIR)/MQTTVersion.o \
	$(OUTDIR)/Socket.o $(OUTDIR)/SocketBuffer.o $(OUTDIR)/SSLSocket.o \
	$(OUTDIR)/StackTrace.o $(OUTDIR)/Thread.o $(OUTDIR)/Tree.o \
	$(OUTDIR)/utf-8.o 
OBJ=$(COMMON_OBJ) $(CFG_OBJ)
ALL_OBJ=$(OUTDIR)/Clients.o $(OUTDIR)/Heap.o $(OUTDIR)/LinkedList.o \
	$(OUTDIR)/Log.o $(OUTDIR)/Messages.o $(OUTDIR)/MQTTAsync.o \
	$(OUTDIR)/MQTTPacket.o $(OUTDIR)/MQTTPacketOut.o \
	$(OUTDIR)/MQTTPersistence.o $(OUTDIR)/MQTTPersistenceDefault.o \
	$(OUTDIR)/MQTTProtocolClient.o $(OUTDIR)/MQTTProtocolOut.o \
	$(OUTDIR)/MQTTVersion.o $(OUTDIR)/Socket.o $(OUTDIR)/SocketBuffer.o \
	$(OUTDIR)/SSLSocket.o $(OUTDIR)/StackTrace.o $(OUTDIR)/Thread.o \
	$(OUTDIR)/Tree.o $(OUTDIR)/utf-8.o 

COMPILE=/usr/local/arm/4.5.1/opt/bin/arm-linux-gcc -c   -g -fPIC -o "$(OUTDIR)/$(*F).o" $(CFG_INC) $<
LINK=/usr/local/arm/4.5.1/opt/bin/arm-linux-gcc  -g -shared -fPIC -o "$(OUTFILE)" $(ALL_OBJ)
COMPILE_ADA=gnat -g -c -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_ADB=gnat -g -c -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_F=gfortran -c -g -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_F90=gfortran -c -g -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_D=gdc -c -g -o "$(OUTDIR)/$(*F).o" "$<"

# Pattern rules
$(OUTDIR)/%.o : ../dep/paho/src/%.c
	$(COMPILE)

$(OUTDIR)/%.o : ../dep/paho/src/%.ada
	$(COMPILE_ADA)

$(OUTDIR)/%.o : ../dep/paho/src/%.d
	$(COMPILE_D)

$(OUTDIR)/%.o : ../dep/paho/src/%.adb
	$(COMPILE_ADB)

$(OUTDIR)/%.o : ../dep/paho/src/%.f90
	$(COMPILE_F90)

$(OUTDIR)/%.o : ../dep/paho/src/%.f
	$(COMPILE_F)

# Build rules
all: $(OUTFILE)

$(OUTFILE): $(OUTDIR)  $(OBJ)
	$(LINK)

$(OUTDIR):
	$(MKDIR) -p "$(OUTDIR)"

# Rebuild this project
rebuild: cleanall all

# Clean this project
clean:
	$(RM) -f $(OUTFILE)
	$(RM) -f $(OBJ)

# Clean this project and all dependencies
cleanall: clean
endif

#
# Configuration: Release
#
ifeq "$(CFG)" "Release"
OUTDIR=Release
OUTFILE=$(OUTDIR)/Mqtt.so
CFG_INC=
CFG_LIB=
CFG_OBJ=
COMMON_OBJ=$(OUTDIR)/Clients.o $(OUTDIR)/Heap.o \
	$(OUTDIR)/LinkedList.o $(OUTDIR)/Log.o $(OUTDIR)/Messages.o \
	$(OUTDIR)/MQTTAsync.o $(OUTDIR)/MQTTPacket.o \
	$(OUTDIR)/MQTTPacketOut.o $(OUTDIR)/MQTTPersistence.o \
	$(OUTDIR)/MQTTPersistenceDefault.o $(OUTDIR)/MQTTProtocolClient.o \
	$(OUTDIR)/MQTTProtocolOut.o $(OUTDIR)/MQTTVersion.o \
	$(OUTDIR)/Socket.o $(OUTDIR)/SocketBuffer.o $(OUTDIR)/SSLSocket.o \
	$(OUTDIR)/StackTrace.o $(OUTDIR)/Thread.o $(OUTDIR)/Tree.o \
	$(OUTDIR)/utf-8.o 
OBJ=$(COMMON_OBJ) $(CFG_OBJ)
ALL_OBJ=$(OUTDIR)/Clients.o $(OUTDIR)/Heap.o $(OUTDIR)/LinkedList.o \
	$(OUTDIR)/Log.o $(OUTDIR)/Messages.o $(OUTDIR)/MQTTAsync.o \
	$(OUTDIR)/MQTTPacket.o $(OUTDIR)/MQTTPacketOut.o \
	$(OUTDIR)/MQTTPersistence.o $(OUTDIR)/MQTTPersistenceDefault.o \
	$(OUTDIR)/MQTTProtocolClient.o $(OUTDIR)/MQTTProtocolOut.o \
	$(OUTDIR)/MQTTVersion.o $(OUTDIR)/Socket.o $(OUTDIR)/SocketBuffer.o \
	$(OUTDIR)/SSLSocket.o $(OUTDIR)/StackTrace.o $(OUTDIR)/Thread.o \
	$(OUTDIR)/Tree.o $(OUTDIR)/utf-8.o 

COMPILE=g++ -c   -fPIC -o "$(OUTDIR)/$(*F).o" $(CFG_INC) $<
LINK=g++  -shared -fPIC -o "$(OUTFILE)" $(ALL_OBJ)
COMPILE_ADA=gnat -O -c -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_ADB=gnat -O -c -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_F=gfortran -O -g -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_F90=gfortran -O -g -o "$(OUTDIR)/$(*F).o" "$<"
COMPILE_D=gdc -c -g -o "$(OUTDIR)/$(*F).o" "$<"

# Pattern rules
$(OUTDIR)/%.o : ../dep/paho/src/%.c
	$(COMPILE)

$(OUTDIR)/%.o : ../dep/paho/src/%.ada
	$(COMPILE_ADA)

$(OUTDIR)/%.o : ../dep/paho/src/%.d
	$(COMPILE_D)

$(OUTDIR)/%.o : ../dep/paho/src/%.adb
	$(COMPILE_ADB)

$(OUTDIR)/%.o : ../dep/paho/src/%.f90
	$(COMPILE_F90)

$(OUTDIR)/%.o : ../dep/paho/src/%.f
	$(COMPILE_F)

# Build rules
all: $(OUTFILE)

$(OUTFILE): $(OUTDIR)  $(OBJ)
	$(LINK)

$(OUTDIR):
	$(MKDIR) -p "$(OUTDIR)"

# Rebuild this project
rebuild: cleanall all

# Clean this project
clean:
	$(RM) -f $(OUTFILE)
	$(RM) -f $(OBJ)

# Clean this project and all dependencies
cleanall: clean
endif