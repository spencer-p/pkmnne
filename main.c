#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>

//name for the savefile
//is a char* instead of #define because i want to add options to change it
char *SAVEFILE = "save.game";

//Typdefs
typedef struct {
    int x;
    int y;
} point;

typedef struct {
    char dat[16][16];
    bool generated;
} block;

//variables shared by everything
point playerblock;
point playerpos;
char name[32];
block **map;
point blocks; //not actually a point but a convienent way to store amt of blocks
int seed;

FILE *logFile;

//ncurses technicalities 
int MAXX, MAXY;

//Prototypes
int readSave();
int writeSave();
void drawMap();
void drawBlock(int by, int bx, int y, int x);
point getPos(int y, int x);
void generateBlock(int by, int bx);
void drawPlayer();

int main() {

    logFile = fopen("log.game", "w");

    //init data
    if (readSave() == 1) {
        printf("Error reading saved data.\n");
        return 1;
    }

    //init ncurses
    initscr();
    raw();
    noecho();
    getmaxyx(stdscr, MAXY, MAXX);
    curs_set(0);

    //colors
    //TODO lol
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    

    char c;

    drawMap();

    while ((c = getch()) != 'Q') {

        erase();

        switch (c) {
            case 'a': //move left
                //decrement x
                if (playerpos.x == 0 && playerblock.x != 0) {
                    playerblock.x--;
                    playerpos.x = 15;
                }
                else {
                    playerpos.x -= (playerpos.x == 0) ? 0 : 1;
                }
                break;
            case 's':
                //move down
                //increment y
                if (playerpos.y == 15) {
                    playerblock.y++;
                    playerpos.y = 0;
                }
                else {
                    playerpos.y++;
                }
                break;
            case 'w':
                //move up
                //decrement y
                if (playerpos.y == 0 && playerblock.y != 0) {
                    playerblock.y--;
                    playerpos.y = 15;
                }
                else {
                    playerpos.y -= (playerpos.y == 0) ? 0 : 1;
                }
                break;
            case 'd':
                //move right
                //increment x
                if (playerpos.x == 15) {
                    playerblock.x++;
                    playerpos.x = 0;
                }
                else {
                    playerpos.x++;
                }
                break;
            case 'S':
                if (writeSave() == 1) {
                    endwin();
                    printf("Save did not work. Have fun.\n");
                    for (int y = 0; y < blocks.y; y++) {
                        free(map[y]);
                    }
                    free(map);
                    return 1;
                }
        }

        if (playerblock.x >= blocks.x || 
                playerblock.y >= blocks.y || 
                map[playerblock.y][playerblock.x].generated == false) {
            generateBlock(playerblock.y, playerblock.x);
        }
        

        drawMap();
    }

    endwin();
    printf("Save? (y/n): ");
    fflush(stdout);
    scanf("%c", &c);
    if (c == 'y') {
        if (writeSave() == 1) {
            printf("Save did not work. Have fun.\n");
        }
        else {
            printf("Successfully saved.\n");
        }
    }
    else {
        printf("Not saving.\n");
    }
    for (int y = 0; y < blocks.y; y++) {
        free(map[y]);
    }
    free(map);
    
    return 0;
}

void generateBlock(int by, int bx) {
    //If the block is outside what's currently allocated,
    //we have to grow the block, and set generated to false
    //so we don't print garbage values
    //NOTE: IF YOU CHANGE X AND Y AT THE SAME TIME THIS WILL BREAK

    fprintf(logFile, "generating block %d %d\n", by, bx);

    if (by >= blocks.y) {
        map = realloc(map, (by+1)*sizeof(block *));
        map[by] = malloc(blocks.x*sizeof(block));
        blocks.y = by+1;
        for (int x = 0; x <= blocks.x; x++) {
            map[by][x].generated = false;
        }
    }
    if (bx >= blocks.x) {
        for (int y = 0; y < blocks.y; y++) {
            map[y] = realloc(map[y], (bx+1)*sizeof(block));
            for (int x = blocks.x; x <= bx; x++) {
                map[y][x].generated = false;
            }
        }
        blocks.x = bx+1;
    }

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int n = rand()%3;
            if (n == 1) {
                map[by][bx].dat[y][x] = 'w';
            }
            else {
                map[by][bx].dat[y][x] = '.';
            }
        }
    }

    map[by][bx].generated = true;
    return;
}

void drawMap() {
    
    point min, max, center;
    center = getPos(playerblock.y, playerblock.x); //for reference
    //This math for which block to use works because
    //it does division over the integers
    //why? so we don't have a ton of conditionals to see if the
    //block we're trying to print would even be on screen.
    //
    //if conditions to check we're not printing too far
    //
    //i hope you never need to work on this
    min.x = playerblock.x-(center.x/16)-1;
    if (min.x < 0) min.x = 0;
    min.y = playerblock.y-(center.y/16)-1;
    if (min.y < 0) min.y = 0;
    max.x = playerblock.x+((MAXX-center.x)/16)+1;
    if (max.x >= blocks.x) max.x = blocks.x;
    max.y = playerblock.y+((MAXY-center.y)/16)+1;
    if (max.y >= blocks.y) max.y = blocks.y;
    
    for (int y = min.y; y < max.y; y++) {
        for (int x = min.x; x < max.x; x++) {
            point start = getPos(y, x);
            drawBlock(y, x, start.y, start.x);
        }
    }

    drawPlayer();

    return;
}

point getPos(int y, int x) {
    point result;
    //centerpixel - distance from center - offset from player coords
    if (x != playerblock.x) {
        result.x = (MAXX/2)-(16*(playerblock.x-x))-playerpos.x;
    }
    else {
        result.x = (MAXX/2)-playerpos.x;
    }
    if (y != playerblock.y) {
        result.y = (MAXY/2)-(16*(playerblock.y-y))-playerpos.y;
    }
    else {
        result.y = (MAXY/2)-playerpos.y;
    }

    return result;
}

void drawPlayer() {
    //wow interesting stuff
    //player is always in the same place
    mvaddch(MAXY/2, MAXX/2, toupper(name[0]) | COLOR_PAIR(4) | A_BOLD);
    return;
}

void drawBlock(int by, int bx, int y, int x) {
    if (map[by][bx].generated == false) {
        return;
    }
    point min, max;
    min.x = (x < 0) ? abs(x) : 0;
    min.y = (y < 0) ? abs(y) : 0;
    max.x = (x+16 >= MAXX) ? 16-((x+16)-MAXX) : 16;
    max.y = (y+16 >= MAXY) ? 16-((y+16)-MAXY) : 16;
    for (int yi = min.y; yi < max.y; yi++) {
        move(y+yi, x+min.x);
        for (int xi = min.x; xi < max.x; xi++) {
                int color = 7;
                switch(map[by][bx].dat[yi][xi]) {
                    case 'w':
                        color = 2; //green
                        break;
                    case '=':
                        color = 3; //yellow
                        break;
                }
                addch(map[by][bx].dat[yi][xi] | COLOR_PAIR(color));
        }
    }
    return;
}

int writeSave() {
    //open up our file for writing
    //is this goes wrong we're very much screwed
    FILE *output = fopen(SAVEFILE, "w");
    if (output == NULL) {
        //bad bad bad bad
        return 1;
    }
    fprintf(output, "playername %s\n", name);
    fprintf(output, "playerblock %d, %d\n", playerblock.y, playerblock.x);
    fprintf(output, "playerpos %d, %d\n", playerpos.y, playerpos.x);
    fprintf(output, "seed %d\n", seed);
    fprintf(output, "blocks %d, %d\n", blocks.y, blocks.x);
    //dear god the quadruple for loop
    for (int y = 0; y < blocks.y; y++) {
        for (int x = 0; x < blocks.x; x++) {
            if (map[y][x].generated == true) {
                fprintf(output, "block [%d %d] ", y, x);
                for (int by = 0; by < 16; by++) {
                    for (int bx = 0; bx < 16; bx++) {
                        fprintf(output, "%c", map[y][x].dat[by][bx]);
                    }
                }
                fprintf(output, "\n");
            }
        }
    }

    fclose(output);
    
    return 0;
}

int readSave() {
    
    //default values
    map = NULL;
    playerblock.x = 0;
    playerblock.y = 0;
    playerpos.x = 0;
    playerpos.y = 0;
    seed = 0;
    blocks.x = 0;
    blocks.y = 0;
    name[0] = 'P'; //for player
    name[1] = '\0';

    //open up our input file
    //might want to change later
    FILE *input = fopen(SAVEFILE, "r");
    if (input == NULL) {
        if (errno != ENOENT) {
            //if the file doesn't exist we ignore and just make up data
            return 1;
        }
    }
    else {
        fprintf(logFile, "reading save\n");
        char buf[64];
        while (fscanf(input, "%s", buf) != EOF) {
            if (strncmp(buf, "playername", 10) == 0) {
                fprintf(logFile, "reading name\n");
                fscanf(input, "%s\n", name);
            }
            else if (strncmp(buf, "seed", 4) == 0) {
                fprintf(logFile, "reading seed\n");
                //seed for map generation
                fscanf(input, "%d\n", &seed);
            }
            else if (strncmp(buf, "blocks", 6) == 0) {
                fprintf(logFile, "allocating memory for blocks\n");
                //makes data structure big enough to contain all the blocks
                //map[y][x]
                fscanf(input, "%d, %d\n", &blocks.y, &blocks.x);
                map = (block **) malloc(blocks.y*sizeof(block *));
                for (int y = 0; y < blocks.y; y++) {
                    map[y] = malloc(blocks.x*sizeof(block));
                    for (int x = 0; x < blocks.x; x++) {
                        map[y][x].generated = false;
                    }
                }
            }
            else if (strncmp(buf, "block", 5) == 0) {
                //oh dear time to load a block
                //256 numbers :(
                if (map == NULL) {
                    //this is bad
                    //don't write data to NULL
                    printf("There is something seriously wrong with your save file.\n");
                    return 1;
                }
                int y, x;
                fscanf(input, " [%d %d] ", &y, &x);
                fprintf(logFile, "reading block %d %d\n", y, x);
                //we know this block is set
                map[y][x].generated = true;
                //simple 2d array writing
                //storing everything as a numbers
                //don't know why. security through obscurity i guess
                for (int yi = 0; yi < 16; yi++) {
                    for (int xi = 0; xi < 16; xi++) {
                        fscanf(input, "%c", &map[y][x].dat[yi][xi]);
                    }
                }
            }
            else if (strncmp(buf, "playerblock", 11) == 0) {
                fprintf(logFile, "reading player's block\n");
                //read the numbers for which block the player is in
                fscanf(input, " %d, %d", &playerblock.y, &playerblock.x);
            }
            else if (strncmp(buf, "playerpos", 9) == 0) {
                fprintf(logFile, "reading player's position\n");
                //read the numbers for where the player is in the block
                fscanf(input, " %d, %d", &playerpos.y, &playerpos.x);
            }
        }
        fclose(input);
    }

    if (seed == 0) {
        fprintf(logFile, "generating seed\n");
        seed = time(NULL);
    }

    srand(seed);
    
    if (map == NULL) {
        generateBlock(0, 0);
    }

    //done
    return 0;
}
