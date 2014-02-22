//functions and data types for the underlying monsters

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
