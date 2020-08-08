PROGRAM = libsimplerl.a
OBJS = path.o map.o queue.o bsp.o rng.o mapgen.o util.o
CFLAGS = -Wall -DRL_TWISTER
LIBFLAGS = -lcurses -lm

$(PROGRAM).a: $(OBJS)
	ar rcs $(PROGRAM) $(OBJS)

%.o: %.c
	gcc -o $@ $(CFLAGS) -c $<

clean:
	rm *.o
	rm $(PROGRAM)
