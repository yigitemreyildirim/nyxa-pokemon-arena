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