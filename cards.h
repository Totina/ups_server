/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020

 *
 */

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
void print_cards(Card *cards, int number_of_cards);

#endif //SP1_CARDS_H
