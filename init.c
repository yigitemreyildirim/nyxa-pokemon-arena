#include "game.h"

static char *createDynamicString(const char *src) {  //verilen metni dinamik olarak kopyaliyor
    size_t len = strlen(src) + 1;
    char *dst = malloc(len);
    if (dst == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(dst, src, len);
    return dst;
}

void initializeTypes(Type types[POKEMON_TYPE_COUNT]) { //types.txt dosyasindaki 18 tipi ve carpanlari okuyor
    FILE *filepointer = fopen("types.txt", "r");
    if (filepointer == NULL) {
        fprintf(stderr, "Could not open types.txt\n");
        exit(EXIT_FAILURE);
    }

    char typeLine[128];
    for (int i = 0; i < POKEMON_TYPE_COUNT; i++) {
        if (fgets(typeLine, sizeof(typeLine), filepointer) == NULL) {
            fprintf(stderr, "Unexpected EOF in types.txt at block %d\n", i);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        typeLine[strcspn(typeLine, "\r\n")] = '\0'; // ilk \r veya \n karakterini bulup yerine '\0' koyarak satır sonunu temizler
        types[i].name = createDynamicString(typeLine);

        for (int j = 0; j < POKEMON_TYPE_COUNT; j++) {
            float mult = 1.0f;
            char def_type[64];
            if (fscanf(filepointer, "%63s %f", def_type, &mult) != 2) {  // buffer overflow onleyici onlem
                fprintf(stderr, "Invalid effect line in types.txt for %s\n", types[i].name);
                fclose(filepointer);
                exit(EXIT_FAILURE);
            }
            types[i].effects[j].atkName = createDynamicString(types[i].name);
            types[i].effects[j].defName = createDynamicString(def_type);
            types[i].effects[j].multiplier = mult;
        }

        types[i].effects[POKEMON_TYPE_COUNT].atkName = createDynamicString(types[i].name); //yeni type pokemon icin
        types[i].effects[POKEMON_TYPE_COUNT].defName = createDynamicString("None");
        types[i].effects[POKEMON_TYPE_COUNT].multiplier = 1.0f;

        int c;
        while ((c = fgetc(filepointer)) != '\n' && c != EOF) {  // yeni ozelligi satir basina goturuyor ve dosya bittiginde hata onler
        }
    }

    fclose(filepointer);
}

static Type *findTypeByName(Type types[POKEMON_TYPE_COUNT], const char *typeName) { // dizide turu ariyor
    for (int i = 0; i < POKEMON_TYPE_COUNT; i++) {
        if (types[i].name != NULL && strcmp(types[i].name, typeName) == 0) {
            return &types[i];
        }
    }
    return NULL;
}

static const Move *findMoveByName(const Move *moves, int moveCount, const char *moveName) {
    for (int i = 0; i < moveCount; i++) {
        if (moves[i].name != NULL && strcmp(moves[i].name, moveName) == 0) {
            return &moves[i];
        }
    }
    return NULL;
}

Move *initializeMoves(Type types[POKEMON_TYPE_COUNT], int *outMoveCount) { 
    FILE *filepointer = fopen("moves.txt", "r");
    if (filepointer == NULL) {
        fprintf(stderr, "Could not open moves.txt\n");
        exit(EXIT_FAILURE);
    }

    Move *moves = NULL;
    int count = 0;
    char line[256];

    while (fgets(line, sizeof(line), filepointer) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        // Bos satırları atla
        if (line[0] == '\0') {
            continue;
        }

        
        char *p1 = strchr(line, '|');  // format: Name|Type|Category|Power|Range
        if (p1 == NULL) {
            fprintf(stderr, "Invalid move format in moves.txt: %s\n", line);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p1 = '\0';
        const char *moveName = line;

        char *typeStr = p1 + 1;
        char *p2 = strchr(typeStr, '|');  //kelime kelime pointerlara kaydettim
        if (p2 == NULL) {
            fprintf(stderr, "Missing type pipe in moves.txt: %s\n", moveName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p2 = '\0';

        char *catStr = p2 + 1;
        char *p3 = strchr(catStr, '|');
        if (p3 == NULL) {
            fprintf(stderr, "Missing category pipe in moves.txt: %s\n", moveName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p3 = '\0';

        char *powerStr = p3 + 1;
        char *p4 = strchr(powerStr, '|');
        if (p4 == NULL) {
            fprintf(stderr, "Missing power pipe in moves.txt: %s\n", moveName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p4 = '\0';

        const char *rangeStr = p4 + 1;
        (void)rangeStr; // range kategoriye gore ataniyor

        Type *matchedType = findTypeByName(types, typeStr);
        if (matchedType == NULL) {
            fprintf(stderr, "Unknown type '%s' for move '%s'\n", typeStr, moveName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }

        AttackType category;
        int range = 0;
        if (strcmp(catStr, "PHYSICAL") == 0) { //fizikselse range 2 
            category = PHYSICAL_DAMAGE;
            range = 2;
        } else if (strcmp(catStr, "SPECIAL") == 0) { // ozelse range 4
            category = SPECIAL_DAMAGE;
            range = 4;
        } else {
            fprintf(stderr, "Invalid category '%s' for move '%s'\n", catStr, moveName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }

        int power = atoi(powerStr);

        Move *resized = realloc(moves, (size_t)(count + 1) * sizeof(Move)); // dinamik array icin alan belirleme
        if (resized == NULL) {
            fprintf(stderr, "Memory allocation failed while reading moves.txt\n");
            for (int k = 0; k < count; k++) {
                free(moves[k].name);
            }
            free(moves);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        moves = resized;

        moves[count].name = createDynamicString(moveName);
        moves[count].type = matchedType;
        moves[count].category = category;
        moves[count].power = power;
        moves[count].range = range;
        count++;
    }

    fclose(filepointer);
    *outMoveCount = count;  //kac tane hamle yapildigini bildirmek icin
    return moves;
}

static void fillTypeEntry(Type *dest, Type types[POKEMON_TYPE_COUNT], const char *typeName) { //tip adina gore default degerler atar 
    if (strcmp(typeName, "None") == 0) {
        dest->name = createDynamicString("None");
        for (int i =  0; i < TOTAL_TYPE_EFFECTS; i++) {  
            dest->effects[i].atkName = createDynamicString("None");
            dest->effects[i].defName = createDynamicString("None");
            dest->effects[i].multiplier = 1.0f;
        }
        return;
    }

    Type *found = findTypeByName(types, typeName);
    if (found == NULL) {
        fprintf(stderr, "Unknown type '%s' in pokemons.txt\n", typeName);
        exit(EXIT_FAILURE);
    }

    dest->name = createDynamicString(found->name); // pokemonun tipini bulup dinamik kopyalayıp hedef pokemonun tipine kaydediyor 
    for (int i= 0; i < TOTAL_TYPE_EFFECTS; i++) {
        dest->effects[i].atkName = createDynamicString(found->effects[i].atkName);
        dest->effects[i].defName = createDynamicString(found->effects[i].defName);
        dest->effects[i].multiplier = found->effects[i].multiplier;
    }
}

Pokemon *initializePokemons(Type types[POKEMON_TYPE_COUNT], Move *moves, int moveCount, int *outPokemonCount) {
    FILE *filepointer = fopen("pokemons.txt", "r");
    if (filepointer == NULL) {
        fprintf(stderr, "Could not open pokemons.txt\n");
        exit(EXIT_FAILURE);
    }

    Pokemon *pokemons = NULL;
    int count =  0;
    char line[512];

    while (fgets(line, sizeof(line), filepointer) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }

        // Format: Name|Type1|Type2|MaxHP|Attack|Defense|SpAtk|SpDef|Speed|Move1|
        char *p1 = strchr(line, '|');
        if (p1 == NULL) {
            fprintf(stderr, "Invalid pokemon format in pokemons.txt: %s\n", line);  //ozelliklerin eksiklik kontrolleri
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p1 = '\0';
        const char *pokeName = line;

        char *type1Str = p1 + 1;
        char *p2 = strchr(type1Str, '|');
        if (p2 == NULL) {
            fprintf(stderr, "Missing Type2 in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p2 = '\0';

        char *type2Str = p2 + 1;
        char *p3 = strchr(type2Str, '|');
        if (p3 == NULL) {
            fprintf(stderr, "Missing MaxHP in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p3 = '\0';

        char *hpStr = p3 + 1;
        char *p4 = strchr(hpStr, '|');
        if (p4 == NULL) {
            fprintf(stderr, "Missing Attack in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p4 = '\0';

        char *atkStr = p4 + 1;
        char *p5 = strchr(atkStr, '|');
        if (p5 == NULL) {
            fprintf(stderr, "Missing Defense in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p5 = '\0';

        char *defStr = p5 + 1;
        char *p6 = strchr(defStr, '|');
        if (p6 == NULL) {
            fprintf(stderr, "Missing SpAtk in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p6 = '\0';

        char *spAtkStr = p6 + 1;
        char *p7 = strchr(spAtkStr, '|');
        if (p7 == NULL) {
            fprintf(stderr, "Missing SpDef in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p7 = '\0';

        char *spDefStr = p7 + 1;
        char *p8 = strchr(spDefStr, '|');
        if (p8 == NULL) {
            fprintf(stderr, "Missing Speed in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p8 = '\0';

        char *spdStr = p8 + 1;
        char *p9 = strchr(spdStr, '|');
        if (p9 == NULL) {
            fprintf(stderr, "Missing moves in pokemons.txt: %s\n", pokeName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        *p9 = '\0';

        Pokemon *resized = realloc(pokemons, (size_t)(count + 1) * sizeof(Pokemon));
        if (resized == NULL) {
            fprintf(stderr, "Memory allocation failed while reading pokemons.txt\n");
            free(pokemons);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }
        pokemons = resized;

        Pokemon *current = &pokemons[count];
        memset(current, 0, sizeof(Pokemon)); // yanlis verilerle karsilasmamak icin verileri temizledim

        current->name = createDynamicString(pokeName);
        fillTypeEntry(&current->types[0], types, type1Str); // ana tipi
        fillTypeEntry(&current->types[1], types, type2Str); // yna ozelligi

        current->maxHP = atoi(hpStr);  //stringleri sayiya cevirme
        current->currentHP = current->maxHP;
        current->attack = atoi(atkStr);
        current->defense = atoi(defStr);
        current->spAtk = atoi(spAtkStr);
        current->spDef = atoi(spDefStr);
        current->speed = atoi(spdStr);

        char *cursor = p9 + 1;
        int moveIdx = 0;
        while (cursor != NULL && *cursor != '\0') { //imlec degiskeniyle pipe ile ayirdigimiz stringi gezip hamleleri kaydetmek icin yaptim
            if (moveIdx >= POKEMON_MOVE_LIMIT) {
                fprintf(stderr, "Too many moves for pokemon '%s'\n", pokeName);
                fclose(filepointer);
                exit(EXIT_FAILURE);
            }
            char *nextPipe = strchr(cursor, '|');
            if (nextPipe != NULL) {
                *nextPipe = '\0';
            }

            const Move *foundMove = findMoveByName(moves, moveCount, cursor);  // yapilan hamleyi buluyor
            if (foundMove == NULL) {
                fprintf(stderr, "Unknown move '%s' for pokemon '%s'\n", cursor, pokeName);
                fclose(filepointer);
                exit(EXIT_FAILURE);
            }

            current->moves[moveIdx] = *foundMove;
            current->moves[moveIdx].name = createDynamicString(foundMove->name);
            moveIdx++;

            if (nextPipe != NULL) {
                cursor = nextPipe + 1;
            }else {
                cursor = NULL;
            }
         }

        count++;
    }

    fclose(filepointer);
    *outPokemonCount = count;
    return pokemons;
}

void freePokemons(Pokemon *pokemons, int pokemonCount) { //pokemonlarla isimiz bittiginde bellegi geri teslim icin
    if (pokemons == NULL) {
        return;
    }

    for (int i = 0; i < pokemonCount; i++) {
        free(pokemons[i].name);

        for (int t = 0; t < 2; t++) {
            free(pokemons[i].types[t].name);
            for (int j = 0; j < TOTAL_TYPE_EFFECTS; j++) {
                free(pokemons[i].types[t].effects[j].atkName);
                free(pokemons[i].types[t].effects[j].defName);
            }
        }

        for (int m = 0; m < POKEMON_MOVE_LIMIT; m++) {
            free(pokemons[i].moves[m].name);
        }
    }

    free(pokemons);
}

void freeTypes(Type types[POKEMON_TYPE_COUNT]) {// bir turun diger ture dayanikliligi icin kullandigimiz type'i isimiz bittiginde bellegi geri teslim icin
    if (types == NULL) {
        return;
    }

    for (int i = 0; i < POKEMON_TYPE_COUNT; i++) {
        free(types[i].name);

        for (int j = 0; j < TOTAL_TYPE_EFFECTS; j++) {
            free(types[i].effects[j].atkName);
            free(types[i].effects[j].defName);
        }
    }
}

void freeMoves(Move *moves, int moveCount) { // salddiri turlerini isimiz bittiginde bellege teslim
    if (moves == NULL) {
        return;
    }

    for (int i = 0; i < moveCount; i++) {
        free(moves[i].name);
    }

    free(moves);
}

void initialize(Type types[POKEMON_TYPE_COUNT], Move **moves, int *outMoveCount,Pokemon **pokemons, int *outPokemonCount,Player *player1, Player *player2) {
    initializeTypes(types);
    *moves = initializeMoves(types, outMoveCount);
    *pokemons = initializePokemons(types, *moves, *outMoveCount, outPokemonCount);

    player1->name = "Player 1";
    player2->name = "Player 2";

    int chosen1[MAX_PARTY_SIZE] = {0};
    int chosen2[MAX_PARTY_SIZE] = {0};

    for (int i = 0; i < MAX_PARTY_SIZE; i++) {
        int index1, duplicate1;  // duplicate degiskeni ayni pokemonu bir daha secemesin diye index1 sectigi pokemon
        do {
            index1 = rand() % (*outPokemonCount);  // pokemon sayisi -1 kadar bir sayi uretir 
            duplicate1 = 0;
            for (int j = 0; j < i; j++) {
                if (chosen1[j] == index1) duplicate1 = 1;
            }
        } while (duplicate1);
        chosen1[i] = index1;
        player1->Pokemons[i] = (*pokemons)[index1];

        int index2, duplicate2;
        do {
            index2 = rand() % (*outPokemonCount); //pokemon sayisi kadar random sayi ureticek
            duplicate2 = 0;
            for (int j = 0; j < i; j++) {
                if (chosen2[j] == index2) duplicate2 = 1;
            }
        } while (duplicate2);
        chosen2[i] = index2;
        player2->Pokemons[i] = (*pokemons)[index2];
    }

    player1->currentIndex = 1;
    player2->currentIndex = 1;

    player1->position.x = 0;
    player1->position.y = 4;

    player2->position.x = 9;
    player2->position.y = 4;
}