simplerl: main.c clean
	gcc -o simplerl -lcurses *.c

clean:
	rm *.o || exit 0
	rm simplerl || exit 0

