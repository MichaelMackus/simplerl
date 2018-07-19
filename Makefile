PROGRAM = simplerl
OBJS = main.o draw.o dungeon.o game.o item.o map.o message.o mob.o path.o random.o tile.o
CCFLAGS = -lcurses

simplerl: $(OBJS)
	gcc -o $(PROGRAM) $(CCFLAGS) $(OBJS)

%.o: %.c %.h
	gcc -c $<

clean:
	rm *.o
	rm $(PROGRAM)
