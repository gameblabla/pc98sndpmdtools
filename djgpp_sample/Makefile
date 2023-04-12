CC = i386-pc-msdosdjgpp-gcc
CFLAGS = -Wall -I. -Isound -Os -fomit-frame-pointer -fdata-sections -ffunction-sections -march=i386 -mtune=i386 -Iinclude
CFLAGS += -I./generic -I./graphics -I. -I./graphics/vga -I./input/dos -Isound/music -Ivideo/pc9821 -Ivideo
LDFLAGS = -lc -Wl,--as-needed -Wl,--gc-sections -flto -s 

OUTPUT = GAME.exe

SOURCES =  ./djgpp_main.c pmd.c
OBJS = ${SOURCES:.c=.o}

all: ${OUTPUT}

${OUTPUT}:${OBJS}
	${CC} -o ${OUTPUT} ${OBJS} ${CFLAGS} ${LDFLAGS} 
	i386-pc-msdosdjgpp-strip --strip-all ${OUTPUT}

clean:
	rm $(OBJS) $(OUTPUT)
