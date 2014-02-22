pkmnne: main.c 
	cc main.c -o pkmnne -lncurses -g -Wall -Werror -pedantic

clean:
	rm log.game save.game pkmnne
