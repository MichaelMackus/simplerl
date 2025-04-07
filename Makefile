PROGRAM = simplerl
SRCS = $(wildcard game/*.c)
OBJS = $(SRCS:%.c=%.o)
CFLAGS = -W -Werror -ggdb -I./
#CFLAGS = -DNCURSES_WIDECHAR=1 -W -Werror -ggdb -I./
LIBFLAGS = -lcurses -lm
#LIBFLAGS = -lcursesw -lm

$(PROGRAM): lib/roguelike.h $(OBJS)
	cc -o $(PROGRAM) $(CFLAGS) $(OBJS) $(LIBFLAGS)

lib/roguelike.h:
	git submodule update

%.o: %.c
	cc -o $@ $(CFLAGS) -c $<

clean:
	rm game/*.o
	rm $(PROGRAM)

test:
	cc -o test $(CFLAGS) test.c $(LIBFLAGS)
