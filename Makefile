PROGRAM = simplerl
OBJS = main.o draw.o dungeon.o game.o item.o map.o message.o mob.o random.o tile.o lib/path.o lib/map.o lib/queue.o lib/bsp.o
CFLAGS = -Wall
LIBFLAGS = -lcurses

simplerl: $(OBJS)
	gcc -o $(PROGRAM) $(CFLAGS) $(LIBFLAGS) $(OBJS)

%.o: %.c
	gcc -o $@ $(CFLAGS) -c $<

clean:
	rm *.o
	rm lib/*.o
	rm $(PROGRAM)
