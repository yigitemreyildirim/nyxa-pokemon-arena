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

        Type *matchedType = findTypeByName(types, typeStr);
        if (matchedType == NULL) {
            fprintf(stderr, "Unknown type '%s' for move '%s'\n", typeStr, moveName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }

        AttackType category;
        if (strcmp(catStr, "PHYSICAL") == 0) { //hamle kategori kontrolu
            category = PHYSICAL_DAMAGE;
        } else if (strcmp(catStr, "SPECIAL") == 0) {
            category = SPECIAL_DAMAGE;
        } else {
            fprintf(stderr, "Invalid category '%s' for move '%s'\n", catStr, moveName);
            fclose(filepointer);
            exit(EXIT_FAILURE);
        }

        int power = atoi(powerStr); //stringleri integere cevirdim
        int range = atoi(rangeStr);

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
