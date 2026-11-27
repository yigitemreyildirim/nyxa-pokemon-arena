#include "game.h"

int calculateDistance(Position pos1, Position pos2) {  // mesafeyi hesabi ve mutlak deger kontrolu 
    int distancex = pos1.x - pos2.x;
    int distancey = pos1.y - pos2.y;

    if (distancex < 0) {
        distancex = -distancex;
    }
    if (distancey < 0) {
        distancey = -distancey;
    }

    return distancex + distancey; //capraz gidemiyorlar
}

void printBoard(const Player *player1, const Player *player2) {
    if (player1 == NULL || player2 == NULL) {
        return;
    }

    char board[ARENA_SIZE][ARENA_SIZE]; //arena olusturdum

    for (int y = 0; y < ARENA_SIZE; y++) {
        for (int x = 0; x < ARENA_SIZE; x++) {
            board[y][x] = '.'; //arena elemanlarina defult deger 
        }
    }

    if (player1->position.x >= 0 && player1->position.x < ARENA_SIZE &&
        player1->position.y >= 0 && player1->position.y < ARENA_SIZE) {
        board[player1->position.y][player1->position.x] = '1';  //oyuncularin arena disina cikmasini engelliyor
    }

    if (player2->position.x >= 0 && player2->position.x < ARENA_SIZE &&
        player2->position.y >= 0 && player2->position.y < ARENA_SIZE) {
        board[player2->position.y][player2->position.x] = '2'; //eger kosul sagliyorsa 2. oyuncu icin 2 degeri yazdiriyor konumunun oldugu yerde
    }

    printf("\n   ");  // gorsel olarak duzgun gozukmesi icin
    for (int x = 0; x < ARENA_SIZE; x++) { //yan yana x in konumlari
        printf("%d ", x);
    }
    printf("\n");

    for (int y = 0; y < ARENA_SIZE; y++) { //alt alta y konumlari
        printf("%d  ", y);
        for (int x = 0; x < ARENA_SIZE; x++) {
            printf("%c ", board[y][x]); //tahtaya degerleri yazdiriyor
        }
        printf("\n");
    }

    const Pokemon *p1ActivePokemon = &player1->Pokemons[player1->currentIndex]; //tahtada olan pokemon
    const Pokemon *p2ActivePokemon= &player2->Pokemons[player2->currentIndex];
    int distance = calculateDistance(player1->position, player2->position);

    printf("\n1: %s | HP %d/%d | (%d,%d)\n",
           p1ActivePokemon->name, p1ActivePokemon->currentHP, p1ActivePokemon->maxHP,
           player1->position.x, player1->position.y);

    printf("2: %s | HP %d/%d | (%d,%d)\n",
           p2ActivePokemon->name, p2ActivePokemon->currentHP, p2ActivePokemon->maxHP,
           player2->position.x, player2->position.y);

    printf("Distance: %d\n\n", distance);
}