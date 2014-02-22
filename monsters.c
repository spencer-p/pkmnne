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
