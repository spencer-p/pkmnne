//functions and data types for the underlying monsters
#include <stdio.h> //for FILE data type

typedef struct {
    char *name;
    int damage;
    int special;
} Attack;

typedef struct {
    int id;
    char name[16];
    Attack attacks[4];
    int maxhealth;
    int health;
    int level;
    int exp;
    int expgoal;
} Monster;

Monster *newMonster(int id);

//Printing function for saving
void mPrint(Monster *self, FILE *output);
Monster *mScan(FILE *input);

//Mutators
void mHeal(Monster *self, int amount);
void mFullHeal(Monster *self);
void mLevelUp(Monster *self);
void mSetName(Monster *self, char *name);
void mAddExp(Monster *self, int exp);

//Getters
char *mGetName(Monster *self);
int mGetMaxHealth(Monster *self);
int mGetHealth(Monster *self);
int mGetLevel(Monster *self);
int mGetExp(Monster *self);
