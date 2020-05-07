PROGRAM = simplerl
OBJS = main.o draw.o dungeon.o game.o item.o map.o message.o mob.o random.o tile.o lib/path.o lib/map.o lib/queue.o
CCFLAGS = -lcurses

simplerl: $(OBJS)
	gcc -o $(PROGRAM) $(CCFLAGS) $(OBJS)

%.o: %.c
	gcc -o $@ -c $<

clean:
	rm *.o
	rm lib/*.o
	rm $(PROGRAM)
