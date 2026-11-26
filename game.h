#ifndef NYXA_GAME_H
#define NYXA_GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PARTY_SIZE 6
#define POKEMON_TYPE_COUNT 18
#define TOTAL_TYPE_EFFECTS 19
#define ROUND_SUMMARY_LEN 200  
#define POKEMON_MOVE_LIMIT 4   
#define ARENA_SIZE 10

typedef struct TypeEffect {
    char *atkName;
    char *defName;
    float multiplier;    // hasar carpani
} TypeEffect;

typedef struct Type {
    char *name;
    TypeEffect effects[TOTAL_TYPE_EFFECTS];
} Type;

typedef enum AttackType {
    PHYSICAL_DAMAGE,
    SPECIAL_DAMAGE
} AttackType;

typedef struct Move {
    char *name;
    Type *type;
    AttackType category;
    int power;
    int range;
} Move;

typedef struct Position {
    int x;
    int y;
} Position;

typedef struct Pokemon {
    char *name;
    Type types[2];
    int maxHP;
    int currentHP;
    int attack;
    int defense;
    int spAtk;
    int spDef;
    int speed;
    Move moves[POKEMON_MOVE_LIMIT];
} Pokemon;

typedef struct Player {
    char *name;
    Pokemon Pokemons[MAX_PARTY_SIZE];
    int currentIndex;
    Position position;
} Player;

typedef struct RoundLog {
    int roundNumber;
    char summary[ROUND_SUMMARY_LEN];
} RoundLog;

// baslatmak icin
void initializeTypes(Type types[POKEMON_TYPE_COUNT]);
Move *initializeMoves(Type types[POKEMON_TYPE_COUNT], int *outMoveCount);
Pokemon *initializePokemons(Type types[POKEMON_TYPE_COUNT], Move *moves, int moveCount, int *outPokemonCount);

// bellek temizleme icin
void freePokemons(Pokemon *pokemons, int pokemonCount);
void freeTypes(Type types[POKEMON_TYPE_COUNT]);
void freeMoves(Move *moves, int moveCount);

#endif