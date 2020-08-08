PROGRAM = simplerl
LIB = libsimplerl.a

lib/$(LIB):
	make -C lib
all: lib/$(LIB)
	make -C game

clean:
	make -C lib clean
	make -C game clean
