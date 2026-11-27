#include <time.h>
#include "game.h"

int main(void) {
    srand((unsigned int)time(NULL)); //saniyeleri baz aliyor boylece her saniye farkli sayi uretiyor

    Type types[POKEMON_TYPE_COUNT];
    Move *moves = NULL;
    int moveCount = 0;
    Pokemon *pokemons = NULL;
    int pokemonCount = 0;
    Player player1;
    Player player2;

    initialize(types, &moves, &moveCount, &pokemons, &pokemonCount, &player1, &player2);

    printf("%s (%d, %d) index=%d\n", player1.name, player1.position.x, player1.position.y, player1.currentIndex);
    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        printf("  [%d] %s\n", i, player1.Pokemons[i].name);
    }

    printf("%s (%d, %d) index=%d\n", player2.name, player2.position.x, player2.position.y, player2.currentIndex);
    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        printf("  [%d] %s\n", i, player2.Pokemons[i].name);
    }

    printBoard(&player1, &player2);

    freePokemons(pokemons, pokemonCount);
    freeMoves(moves, moveCount);
    freeTypes(types);

    return 0;
}