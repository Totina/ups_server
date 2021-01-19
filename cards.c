//
// Created by terez on 1/16/2021.
//

#include <stdio.h>
#include "string.h"
#include <stdlib.h>
#include <time.h>

#include "cards.h"

/**
 * Funkce, naplní balíček kartami.
 *
 * @return nový naplněný balíček kartami - Card *
 */
Card *prepare_cards(){
    Card *cards = (Card*)malloc(NUMBER_OF_CARDS * sizeof(Card));

    // pattern
    for(int i = 0; i < 32; i++) {
        if(i < 8) {
            cards[i].pattern = 1;
        }
        else if(i < 16) {
            cards[i].pattern = 2;
        }
        else if(i < 24) {
            cards[i].pattern = 3;
        }
        else {
            cards[i].pattern = 4;
        }

        // sedm
        if(i == 0 || i == 8 || i == 16 || i == 24) {
            strcpy(cards[i].name, "7");
            cards[i].value = 7;
        }
        // osm
        if(i == 1 || i == 9 || i == 17 || i == 25) {
            strcpy(cards[i].name, "8");
            cards[i].value = 8;
        }
        // devet
        if(i == 2 || i == 10 || i == 18 || i == 26) {
            strcpy(cards[i].name, "9");
            cards[i].value = 9;
        }
        // deset
        if(i == 3 || i == 11 || i == 19 || i == 27) {
            strcpy(cards[i].name, "10");
            cards[i].value = 10;
        }
        // eso
        if(i == 4 || i == 12 || i == 20 || i == 28) {
            strcpy(cards[i].name, "eso");
            cards[i].value = 11;
        }
        // kral
        if(i == 5 || i == 13 || i == 21 || i == 29) {
            strcpy(cards[i].name, "kral");
            cards[i].value = 2;
        }
        // svrsek
        if(i == 6 || i == 14 || i == 22 || i == 30) {
            strcpy(cards[i].name, "svrsek");
            cards[i].value = 1;
        }
        // spodek
        if(i == 7 || i == 15 || i == 23 || i == 31) {
            strcpy(cards[i].name, "spodek");
            cards[i].value = 1;
        }
    }

    // shuffle
    srand(time(0));
    Card tmp;
    //Card *tmp = (Card*)malloc(sizeof(Card));

    for (int i = 0; i < NUMBER_OF_CARDS; i++){
        // random position
        int r = i + (rand() % (NUMBER_OF_CARDS - i));
        // swap
        tmp = cards[i];
        cards[i] = cards[r];
        cards[r] = tmp;
    }
    //free(&tmp);

    return cards;
}


/**
 * Vypíše informaci o kartach
 *
 * @param list_of_games  List místnotí
 */
void print_cards(Card *cards) {

    for(int i = 0; i < NUMBER_OF_CARDS; i++) {
        printf("Card ");
        printf("name: %s ", cards[i].name);
        printf("value:%d ", cards[i].value);
        printf("pattern: %d\n", cards[i].pattern);
    }

    printf("\n");
}