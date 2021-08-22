PROGRAM = simplerl
LIB = libsimplerl.a

all:
	make -C lib
	make -C game

lib/$(LIB):
	make -C lib

clean:
	make -C lib clean
	make -C game clean
