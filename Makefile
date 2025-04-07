PROGRAM = simplerl
SRCS = $(wildcard game/*.c)
OBJS = $(SRCS:%.c=%.o)
CFLAGS = -W -Werror -ggdb -I./
#CFLAGS = -DNCURSES_WIDECHAR=1 -W -Werror -ggdb -I./
LIBFLAGS = -lcurses -lm
#LIBFLAGS = -lcursesw -lm

$(PROGRAM): $(OBJS)
	gcc -o $(PROGRAM) $(CFLAGS) $(OBJS) $(LIBFLAGS)

%.o: %.c
	gcc -o $@ $(CFLAGS) -c $<

clean:
	rm game/*.o
	rm $(PROGRAM)

test:
	gcc -o test $(CFLAGS) test.c $(LIBFLAGS)
