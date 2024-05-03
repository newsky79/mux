CC = gcc
CPP = g++
CFLAGS = -g
TARGET = ./helloworld

%.o: %.c
	$(CC) $(CFLAGS) -DOS_ENVIRONMENT_LINUX -DMUX_DEBUG_SUPPORT -c $< -I../inc -I /usr/include -I ./../LinuxMux/ws -o $@
	
%.o:%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@
	
SOURCES = $(wildcard *.c *.cpp)
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

$(TARGET) : $(OBJS)
	$(CPP) $(OBJS) -g -o $(TARGET) -I./../LinuxMux/ws -L./../LinuxMux/ws -lmux -L/usr/lib -lpthread -lpthread_nonshared -lrt
	
	chmod a+x $(TARGET)

clean:
	rm -rf *.o helloworld

