# simplerl

NOTE: I've moved the library code here:
https://github.com/MichaelMackus/libroguelike It is now a single header library
with a much cleaner interface. A rewrite of this game is pending with the new
library.

This is a simple roguelike library, with an example game implementation.
The library is a collection of tools such as dungeon generation,
pathfinding, and more things that are useful for roguelikes and/or RPGs.

# Building

To build the library simply run `make`. To build the library and game
you run `make all`.

The only dependency for the library are libm (which is part of the C
standard library and should be included by default on your platform).
The only dependencies for the game are libm and libcurses. Windows
building *should* work, but this has been untested and might require
further tweaking (please create any relevant github issues).

The resulting (static) library will be built at lib/libsimplerl.a and
the game will be built at game/simplerl
