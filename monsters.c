#include "monsters.h"
#include <string.h>
#include <stdlib.h>

Monster *initMonster(int id, char *name) {

    Monster *new = calloc(1, sizeof(Monster));

    new->id = id; //copy the id
    strncpy(new->name, name, 16); //copy the name

    switch (id) {
        case 0: //ridiculous debug/test monster
            new->maxhealth = 10000000; //this thing is never dying
            new->health = new->maxhealth;
            new->attacks[0].damage = 10000; //pew pew
            new->attacks[0].name = "OP";
            new->attacks[1].damage = 10; //for beating up whatever
            new->attacks[1].name = "meh";
            break;
        case 1: //weak ass shit thing
            new->maxhealth = 30;
            new->health = new->maxhealth;

            new->attacks[0].damage = 5;
            new->attacks[0].name = "Scratch";
            new->attacks[1].damage = 0;
            new->attacks[1].name = "Splash";
            new->attacks[2].damage = 0;
            new->attacks[2].special = 1;
            new->attacks[2].name = "Rawr";

            break;
    }

    return new;
}

void mPrint(Monster *self, FILE *output) {
    fprintf(output, "Monster\n");
    fprintf(output, "id %d ", self->id);
    fprintf(output, "name %s ", self->name);
    fprintf(output, "maxhealth %d ", self->maxhealth);
    fprintf(output, "health %d ", self->health);
    fprintf(output, "level %d ", self->level);
    fprintf(output, "exp %d ", self->exp);
    fprintf(output, "expgoal %d ", self->expgoal);
    for (int i = 0; i < 4; i++) {
        if (self->attacks[i].name != NULL) {
            fprintf(output, "Attack %s %d %d ", self->attacks[i].name, self->attacks[i].damage, self->attacks[i].special);
        }
    }
    fprintf(output, "end\n");

    return;
}

Monster *mScan(FILE *input) {
    Monster *self = calloc(1, sizeof(Monster));
    char buf[16];
    while (fscanf(input, "%s", buf) != EOF) {
        if (strncmp(buf, "id", 2) == 0) {
            fscanf(input, "%d", &self->id);
        }
        else if (strncmp(buf, "name", 4) == 0) {
            fscanf(input, "%s", self->name); //WARNING: MAY OVERFLOW
        }
        else if (strncmp(buf, "maxhealth", 9) == 0) {
            fscanf(input, "%d", &self->maxhealth);
        }
        else if (strncmp(buf, "health", 6) == 0) {
            fscanf(input, "%d", &self->health);
        }
        else if (strncmp(buf, "level", 5) == 0) {
            fscanf(input, "%d", &self->level);
        }
        else if (strncmp(buf, "expgoal", 7) == 0) {
            fscanf(input, "%d", &self->expgoal);
        }
        else if (strncmp(buf, "exp", 3) == 0) {
            fscanf(input, "%d", &self->exp);
        }
        else if (strncmp(buf, "Attack", 6) == 0) {
            for (int i = 0; i < 4; i++) {
                if (self->attacks[i].name == NULL) {
                    self->attacks[i].name = malloc(16*sizeof(char));
                    fscanf(input, " %s %d %d", self->attacks[i].name, &self->attacks[i].damage, &self->attacks[i].special);
                    break;
                }
            }
        }
        else if (strncmp(buf, "end", 3) == 0) {
            break;
        }
    }
    return self;
}

//Mutators

void mHeal(Monster *self, int amount) {
    self->health += amount;
    if (self->health > self->maxhealth || self->health < 0) {
        //if self->health is less than 0 i'm actually impressed
        self->health = self->maxhealth;
    }
    return;
}

void mFullHeal(Monster *self) {
    self->health = self->maxhealth;
}

void mLevelUp(Monster *self) {
    //TODO
    self->level++;
}

void mSetName(Monster *self, char *name) {
    strncpy(self->name, name, 16);
    return;
}

void mAddExp(Monster *self, int exp) {
    self->exp += exp;
    if (self->exp > self->expgoal) {
        mLevelUp(self);
    }
    return;
}

//Getters
char *mGetName(Monster *self) {
    return self->name;
}

int mGetMaxHealth(Monster *self) {
    return self->maxhealth;
}

int mGetHealth(Monster *self) {
    return self->health;
}

int mGetLevel(Monster *self) {
    return self->level;
}

int mGetExp(Monster *self) {
    return self->exp;
}
