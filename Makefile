CC=msp430-gcc
CFLAGS=-Os -Wall -g -mmcu=msp430g2553

LCD_OBJS=lcd.o
ROTARY_OBJ=rotary.o
.PHONY : clean
OBJS=${LCD_OBJS} ${ROTARY_OBJ}

all: rotary lcd

rotary: $(ROTARY_OBJ)
	$(CC) $(CFLAGS) -Wa,-ahl=rotary.lst -o rotary.elf $(ROTARY_OBJ)

lcd: $(LCD_OBJS)
	$(CC) $(CFLAGS) -Wa,-ahl=lcd.lst -o lcd.elf $(LCD_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm -f lcd.elf rotary.elf *.o
