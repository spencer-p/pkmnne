pkmnne: main.c monsters.c monsters.h
	cc main.c monsters.c -o pkmnne -lncurses -g -Wall -Werror -pedantic
clean:
	rm log.game save.game pkmnne
