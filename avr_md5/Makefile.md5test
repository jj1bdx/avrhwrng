SRC   	= md5test.c md5.c md5.S
OBJ   	= $(SRC:.c=.o)
OUT     = md5test
CFLAGS 	= -Os -DF_CPU=16000000UL -mmcu=atmega328p
GCC   	= avr-gcc
NONUSED = 

$(OUT): $(OBJ)
	$(GCC) $(CFLAGS) $(OBJ) -o $(OUT)

%.o: %.c
	$(GCC) $(CFLAGS) $< -c -o $@

clean:
	del $(OBJ)

