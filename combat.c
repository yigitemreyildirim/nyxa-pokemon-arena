#include "game.h"

static float getTypeMultiplier(const Type *atkType, const char *defTypeName) {
    if (atkType == NULL || defTypeName == NULL || strcmp(defTypeName, "None") == 0) { // hata onlemek icin
        return 1.0f;
    }

    for (int i = 0; i < TOTAL_TYPE_EFFECTS; i++) {
        if (atkType->effects[i].defName != NULL &&
            strcmp(atkType->effects[i].defName, defTypeName) == 0) {
            return atkType->effects[i].multiplier;  //degerleri karsilastirip carpan sayisini buluyor
        }
    }

    return 1.0f;
}

static float getStabMultiplier(const Pokemon *attacker, const Move *move) { // same type attack bonus
    if (attacker == NULL || move == NULL || move->type == NULL || move->type->name == NULL) {
        return 1.0f;
    }

    const char *moveTypeName = move->type->name;
    if ((attacker->types[0].name != NULL && strcmp(attacker->types[0].name, moveTypeName) == 0) ||
        (attacker->types[1].name != NULL && strcmp(attacker->types[1].name, moveTypeName) == 0)) {
        return 1.5f;  //ayni tur attack kullaniydiysa 1.5 kat artiyor hasar
    }

    return 1.0f;
}

int isValidMoveTarget(const Player *player, const Player *otherPlayer, Position target) { //hamle gecerli mi kontrolu
    if (player == NULL || otherPlayer == NULL) {
        return 0;
    }

    if (target.x < 0 || target.x >= ARENA_SIZE ||
        target.y < 0 || target.y >= ARENA_SIZE) {
        return 0;
    }

    if (target.x == otherPlayer->position.x &&
        target.y == otherPlayer->position.y) {
        return 0;
    }

    int dist = calculateDistance(player->position, target);
    if (dist < 1 || dist > 3) { //2 blockluk mesafede degilse engellemek icin
        return 0;
    }

    return 1;
}

int applyDamage(Pokemon *attacker, Pokemon *defender, const Move *move,Position attackerPosition, Position defenderPosition) {  //hasar fonksiyonu
    if (attacker == NULL || defender == NULL || move == NULL) {
        return 0;
    }

    int dist = calculateDistance(attackerPosition, defenderPosition);
    if (dist > move->range) {
        printf("%s is out of range! (distance %d, range %d)\n",
               move->name, dist, move->range); //menzil kontrolu icin
        return 0;
    }

    float atkStat = 0.0f;
    float defStat = 1.0f;//defult degerler 0 a bolunmemesi icin onlem

    if (move->category == PHYSICAL_DAMAGE) {
        atkStat = (float)attacker->attack;

        if (defender->defense > 0) {
            defStat = (float)defender->defense;
        } else {
            defStat = 1.0f;
        }
    } else {
        atkStat = (float)attacker->spAtk;

        if (defender->spDef > 0) {
            defStat = (float)defender->spDef;
        } else {
            defStat = 1.0f;
        }
    }

    float typeEffect1 = getTypeMultiplier(move->type, defender->types[0].name);
    float typeEffect2 = getTypeMultiplier(move->type, defender->types[1].name);
    float stab = getStabMultiplier(attacker, move);

    float rawDamage = (float)move->power * (atkStat / defStat) * typeEffect1 * typeEffect2 * stab;  // belgedeki formulu uyguladim
    int damage = (int)rawDamage;

    if (damage < 0) { //damage negatiflik kontrolu
        damage = 0;
    }

    defender->currentHP -= damage;
    if (defender->currentHP < 0) {  // negatif can gozukmemesi icin
        defender->currentHP = 0;
    }

    return damage;
}