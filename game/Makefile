PROGRAM = simplerl
OBJS = main.o draw.o dungeon.o game.o item.o map.o message.o mob.o random.o tile.o
CFLAGS = -Wall -DRL_TWISTER -I../
LIBFLAGS = -lcurses -lm -L../lib -lsimplerl

$(PROGRAM): $(OBJS)
	gcc -o $(PROGRAM) $(CFLAGS) $(OBJS) $(LIBFLAGS)

%.o: %.c
	gcc -o $@ $(CFLAGS) -c $<

clean:
	rm *.o
	rm $(PROGRAM)
