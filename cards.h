//
// Created by terez on 1/16/2021.
//

#ifndef SP1_CARDS_H
#define SP1_CARDS_H

#define NUMBER_OF_CARDS 32

typedef enum pattern {
    SRDCE,
    KULE,
    ZALUDY,
    LISTY
} pattern;

typedef struct Card {
    char name[6];
    int value;
    pattern pattern;
} Card;

Card *prepare_cards();
void print_cards(Card *cards);

#endif //SP1_CARDS_H
