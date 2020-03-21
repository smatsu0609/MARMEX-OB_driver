CC = gcc
CFLAGS = -Wall
OBJS = oled.o oled_port.o font.o 
LIBFILE = liboled.a


.PHONY: all clean scrub

all: $(LIBFILE)

$(LIBFILE): $(OBJS)
	ar rcs $(LIBFILE) $(OBJS)

clean:
	rm -f $(OBJS)

scrub: clean
	rm -f $(LIBFILE)
