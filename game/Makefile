PROGRAM = simplerl
OBJS = main.o draw.o dungeon.o game.o item.o message.o mob.o random.o
CFLAGS = -W -DRL_TWISTER -I../
LIBFLAGS = -lcurses -lm -L../lib -lsimplerl

$(PROGRAM): $(OBJS)
	gcc -o $(PROGRAM) $(CFLAGS) $(OBJS) $(LIBFLAGS)

%.o: %.c
	gcc -o $@ $(CFLAGS) -c $<

clean:
	rm *.o
	rm $(PROGRAM)

test:
	gcc -o test $(CFLAGS) test.c $(LIBFLAGS)
