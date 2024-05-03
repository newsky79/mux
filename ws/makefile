##########################################################################
# File:             makefile
# Version:	        
# Author:           
# DATE              NAME                      DESCRIPTION
##########################################################################

MUX_COMPILE_OPT = -g

###############################################################################

MUX_INCDIR = ../inc

MUX_SRCDIR = ../src

MUX_LIBDIR = ../ws

###############################################################################

MUX_OBJECTS =     $(MUX_SRCDIR)/mux_api.o \
                  $(MUX_SRCDIR)/mux_dlc.o \
                  $(MUX_SRCDIR)/mux_dlc_ctrl.o \
                  $(MUX_SRCDIR)/mux_frame.o \
                  $(MUX_SRCDIR)/mux_packet.o \
                  $(MUX_SRCDIR)/mux_timer.o \
                  $(MUX_SRCDIR)/mux_uart.o \
                  $(MUX_SRCDIR)/mux_util.o \
                  $(MUX_SRCDIR)/mux_os_thread.o \
                  $(MUX_SRCDIR)/mux_os_sema.o \
                  $(MUX_SRCDIR)/mux_os_mem.o \
                  $(MUX_SRCDIR)/mux_os_mutex.o \
                  $(MUX_SRCDIR)/mux_os_event.o \
                  $(MUX_SRCDIR)/mux_os_timer.o \
                  $(MUX_SRCDIR)/mux_os_util.o \
                  $(MUX_SRCDIR)/com_dev.o
 #                 $(MUX_SRCDIR)/test.o

###############################################################################

MUX_INCLUDE = -I../inc -I /usr/include



###############################################################################

MUX_FLAGS = -DOS_ENVIRONMENT_LINUX -DMUX_DEBUG_SUPPORT


###############################################################################

MUX_TARGET = libmux.a


###############################################################################

all: $(MUX_OBJECTS)
	-mkdir $(MUX_LIBDIR)
	ar -r $(MUX_LIBDIR)/$(MUX_TARGET) $(MUX_OBJECTS)
	cp -raf $(MUX_INCDIR)/mux_api.h $(MUX_LIBDIR)/mux_api.h
	cp -raf $(MUX_INCDIR)/mux_type.h $(MUX_LIBDIR)/mux_type.h
	cp -raf $(MUX_INCDIR)/mux_os_type.h $(MUX_LIBDIR)/mux_os_type.h
	cp -raf $(MUX_INCDIR)/mux_os_api.h $(MUX_LIBDIR)/mux_os_api.h

$(MUX_OBJECTS): $(MUX_SRCDIR)/%.o: $(MUX_SRCDIR)/%.c
	gcc -c $(MUX_COMPILE_OPT) $(MUX_FLAGS) $(MUX_INCLUDE) $< -o $@

###############################################################################

clean: 
	rm -rf $(MUX_OBJECTS)
	rm -rf $(MUX_LIBDIR)/$(MUX_TARGET)
	rm -rf $(MUX_LIBDIR)/mux_api.h
	rm -rf $(MUX_LIBDIR)/mux_type.h
	rm -rf $(MUX_LIBDIR)/mux_os_type.h
	rm -rf $(MUX_LIBDIR)/mux_os_api.h

