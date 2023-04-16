CC = wcl
MODE = c

DEFINES = -0 -ox -bcl=dos -m$(MODE) -s -I. -DTARGET_MSDOS=16

CFLAGS += $(DEFINES)

OUTPUT = TESTSND.EXE

SOURCES =  ./main.c

all: ${OUTPUT}


${OUTPUT}: ${SOURCES}
	${CC} ${CFLAGS} -fe=${OUTPUT} ${SOURCES} 
	rm *.o *.err
	
clean:
	rm *.o ${OUTPUT} *.err
