CC = cc
CFLAGS = -Wall -Wextra -lm -ldl
TARGET = main list.so degree2.so

build: ${TARGET}

main: main.c
	${CC} ${CFLAGS} -o $@ $<

blocks.so: blocks.c
	${CC} ${CFLAGS} -shared -fPIC -o $@ $<

doubles.so: doubles.c
	${CC} ${CFLAGS} -shared -fPIC -o $@ $<


clean:
	@rm -f ${TARGET}
	@rm -rf ./*.dSYM
