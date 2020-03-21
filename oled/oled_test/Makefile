CC = gcc
MAKEOPTS = --no-print-directory
CFLAGS = -Wall
CFLAGS += $(foreach dir, $(LIBDIRS), -I$(LIBBASE)/$(dir))
LINKOPTS = $(foreach dir, $(LIBDIRS), -L$(LIBBASE)/$(dir))

OBJS = oled_test.o bitmap.o
TARGET = oled_test
LIBBASE = ..
LIBDIRS = lib
LIBS = -loled


.PHONY: all target clean scrub $(LIBDIRS)

all: $(LIBDIRS) target

target: $(OBJS)
	$(CC) $(CFLAGS) $(LINKOPTS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo 
	@echo Build success
	@echo 

clean: $(LIBDIRS)
	rm -f $(OBJS)

scrub: $(LIBDIRS) clean
	rm -f $(OBJS) $(TARGET)

$(LIBDIRS):
	$(MAKE) $(MAKEOPTS) -C $(LIBBASE)/$@ $(MAKECMDGOALS)

