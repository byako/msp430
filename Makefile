CC=msp430-gcc
CFLAGS=-Os -Wall -g -mmcu=msp430g2452

RPI_OBJS=rpi.o
ROTARY_OBJ=rotary.o


all: rotary rpi

rotary: $(ROTARY_OBJ)
	$(CC) $(CFLAGS) -Wa,-ahl=rotary.lst -o rotary.elf $(ROTARY_OBJ)

rpi: $(RPI_OBJS)
	$(CC) $(CFLAGS) -Wa,-ahl=rpi.lst -o rpi.elf $(RPI_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -fr rpi.elf rotary.elf $(OBJS)
