#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>

#include "monsters.h"

//name for the savefile
//is a char* instead of #define because i want to add options to change it
char *SAVEFILE = "save.game";

//Typdefs
typedef struct {
    int x;
    int y;
} point;

typedef struct {
    char dat[8][8];
    bool generated;
} block;

//variables shared by everything
struct {
    point block;
    point pos;
    char name[32];
    Monster *monsters[4];
} player;
//point playerblock;
//point playerpos;
//char name[32];
block **map;
point blocks; //not actually a point but a convienent way to store amt of blocks
int seed;

block nullblock; //for simplicity

FILE *logFile;

//Prototypes
int readSave();
int writeSave();
void drawMap();
void drawBlock(int by, int bx, int y, int x);
point getPos(int y, int x);
void generateBlock(int by, int bx);
void drawPlayer();
block *getBlock(int y, int x); //lol stupid wrapper function to avoid segfaults

int main() {

    logFile = fopen("log.game", "w");

    //init data
    if (readSave() == 1) {
        printf("Error reading saved data.\n");
        return 1;
    }
    //init the "nullblock"
    nullblock.generated = false;
    for (int i = 0; i < 8; i++) {
        memset(nullblock.dat[i], '\0', 8*sizeof(char));
    }

    //init ncurses
    initscr();
    raw();
    noecho();
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
                if (player.pos.x == 0 && player.block.x != 0) {
                    player.block.x--;
                    player.pos.x = 15;
                }
                else {
                    player.pos.x -= (player.pos.x == 0) ? 0 : 1;
                }
                break;
            case 's':
                //move down
                //increment y
                if (player.pos.y == 15) {
                    player.block.y++;
                    player.pos.y = 0;
                }
                else {
                    player.pos.y++;
                }
                break;
            case 'w':
                //move up
                //decrement y
                if (player.pos.y == 0 && player.block.y != 0) {
                    player.block.y--;
                    player.pos.y = 15;
                }
                else {
                    player.pos.y -= (player.pos.y == 0) ? 0 : 1;
                }
                break;
            case 'd':
                //move right
                //increment x
                if (player.pos.x == 15) {
                    player.block.x++;
                    player.pos.x = 0;
                }
                else {
                    player.pos.x++;
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

        if (player.block.x >= blocks.x || 
                player.block.y >= blocks.y || 
                map[player.block.y][player.block.x].generated == false) {
            generateBlock(player.block.y, player.block.x);
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

    //begin block allocation
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
    for (int y = 0; y < 8; y++) {
        memset(map[by][bx].dat[y], '\0', 8*sizeof(char));
    }
    //end block allocation

	//Seed randomization with seed AND coordinates, so order doesn't matter
	srand(seed*(by+1)*(bx+1));
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (map[by][bx].dat[y][x] == '\0') {
				int r = rand()%3;
				switch (r) {
					case 0:
						map[by][bx].dat[y][x] = 'w';
						break;
					default:
						map[by][bx].dat[y][x] = '.';
						break;
				}
            }
        }
    }

    map[by][bx].generated = true;
    return;
}

void drawMap() {
    
    point min, max, center;
    center = getPos(player.block.y, player.block.x); //for reference
    //This math for which block to use works because
    //it does division over the integers
    //why? so we don't have a ton of conditionals to see if the
    //block we're trying to print would even be on screen.
    //
    //if conditions to check we're not printing too far
    //
    //i hope you never need to work on this
    min.x = player.block.x-(center.x/16)-1;
    if (min.x < 0) min.x = 0;
    min.y = player.block.y-(center.y/16)-1;
    if (min.y < 0) min.y = 0;
    max.x = player.block.x+((COLS-center.x)/16)+1;
    if (max.x >= blocks.x) max.x = blocks.x;
    max.y = player.block.y+((LINES-center.y)/16)+1;
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
    if (x != player.block.x) {
        result.x = (COLS/2)-(16*(player.block.x-x))-player.pos.x;
    }
    else {
        result.x = (COLS/2)-player.pos.x;
    }
    if (y != player.block.y) {
        result.y = (LINES/2)-(16*(player.block.y-y))-player.pos.y;
    }
    else {
        result.y = (LINES/2)-player.pos.y;
    }

    return result;
}

void drawPlayer() {
    //wow interesting stuff
    //player is always in the same place
    mvaddch(LINES/2, COLS/2, toupper(player.name[0]) | COLOR_PAIR(4) | A_BOLD);
    return;
}

void drawBlock(int by, int bx, int y, int x) {
    if (map[by][bx].generated == false) {
        return;
    }
    for (int yi = 0; yi < 16; yi += 2) { // 16x16 sections, 2x2 chunks of each section
        for (int xi = 0; xi < 16; xi += 2) {
            for (int yii = 0; yii < 2; yii++) { //2x2
                for (int xii = 0; xii < 2; xii++) {
                    point pos;
                    pos.x = x+xi+xii; // Simply calculates the cursor position and checks if on screen
                    pos.y = y+yi+yii;
                    if (pos.x >= 0 && pos.x < COLS && pos.y >= 0 && pos.y < LINES) {
                        
                        int tile = map[by][bx].dat[yi/2][xi/2]; //so we can randomize for more depth
                        int color = 7;
                        switch(map[by][bx].dat[yi/2][xi/2]) { //switch case for deciding colors
                            case 'w':
                                color = 2; //green
                                if (rand()%9 == 0) {
                                    tile = 'm';
                                }
                                break;
                            case '=':
                                color = 3; //yellow
                                break;
                        }

                        move(y+yi+yii, x+xi+xii);
                        addch(tile | COLOR_PAIR(color));
                    }
                }
            }
        }
    }
    /*point min, max;
    min.x = (x < 0) ? abs(x) : 0;
    min.y = (y < 0) ? abs(y) : 0;
    max.x = (x+16 >= COLS) ? 16-((x+16)-COLS) : 16; //i don't know what these lines do
    max.y = (y+16 >= LINES) ? 16-((y+16)-LINES) : 16;
    for (int yi = min.y; yi < max.y; yi += 2) { //inc y by 2 because we draw 2x2 for every char in the map
        move(y+yi, x+min.x); 
        for (int xi = min.x; xi < max.x; xi += 2) {
                int color = 7;
                switch(map[by][bx].dat[yi/2][xi/2]) {
                    case 'w':
                        color = 2; //green
                        break;
                    case '=':
                        color = 3; //yellow
                        break;
                }
                for (int yii = 0; yii < 2; yii++) {
                    move (y+yi+yii, x+xi+min.x);
                    for (int xii = 0; xii < 2; xii++) {
                        addch(map[by][bx].dat[yi/2][xi/2] | COLOR_PAIR(color));
                    }
                }
        }
    }*/
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
    fprintf(output, "playername %s\n", player.name);
    fprintf(output, "playerblock %d, %d\n", player.block.y, player.block.x);
    fprintf(output, "playerpos %d, %d\n", player.pos.y, player.pos.x);
    fprintf(output, "seed %d\n", seed);
    fprintf(output, "blocks %d, %d\n", blocks.y, blocks.x);
    //dear god the quadruple for loop
    for (int y = 0; y < blocks.y; y++) {
        for (int x = 0; x < blocks.x; x++) {
            if (map[y][x].generated == true) {
                fprintf(output, "block [%d %d] ", y, x);
                for (int by = 0; by < 8; by++) {
                    for (int bx = 0; bx < 8; bx++) {
                        fprintf(output, "%c", map[y][x].dat[by][bx]);
                    }
                }
                fprintf(output, "\n");
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        if (player.monsters[i] != NULL) {
            mPrint(player.monsters[i], output);
        }
    }

    fclose(output);
    
    return 0;
}

int readSave() {
    
    //default values
    map = NULL;
    player.block.x = 0;
    player.block.y = 0;
    player.pos.x = 0;
    player.pos.y = 0;
    seed = 0;
    blocks.x = 0;
    blocks.y = 0;
    player.name[0] = 'P'; //for player
    player.name[1] = '\0';
    for (int i = 0; i < 4; i++) {
        player.monsters[i] = NULL;
    }

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
                fscanf(input, "%s\n", player.name);
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
                for (int yi = 0; yi < 8; yi++) {
                    for (int xi = 0; xi < 8; xi++) {
                        fscanf(input, "%c", &map[y][x].dat[yi][xi]);
                    }
                }
            }
            else if (strncmp(buf, "playerblock", 11) == 0) {
                fprintf(logFile, "reading player's block\n");
                //read the numbers for which block the player is in
                fscanf(input, " %d, %d", &player.block.y, &player.block.x);
            }
            else if (strncmp(buf, "playerpos", 9) == 0) {
                fprintf(logFile, "reading player's position\n");
                //read the numbers for where the player is in the block
                fscanf(input, " %d, %d", &player.pos.y, &player.pos.x);
            }
            else if (strncmp(buf, "Monster", 7) == 0) {
                fprintf(logFile, "reading a monster\n");
                for (int i = 0; i < 4; i++) {
                    if (player.monsters[i] == NULL) {
                        player.monsters[i] = mScan(input);
                        break;
                    }
                }
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

block *getBlock(int y, int x) {
    if (y < 0 || x < 0 || y >= blocks.x || x >= blocks.x) {
        return &nullblock;
    }
    else if (map[y][x].generated == false) {
        return &nullblock;
    }
    else {
        return &map[y][x];
    }
}
