/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020

 *
 */

#ifndef SP1_GAME_H
#define SP1_GAME_H

#include "client.h"
#include "cards.h"

#define NUMBER_OF_CARDS 32

typedef enum game_state {
    GAME_STATE_NONE,
    GAME_STATE_WAITING,
    GAME_STATE_FULL,
    GAME_STATE_IN_GAME,
    GAME_STATE_FINISHED
} game_state;

typedef struct Game {
    int id_game;
    enum game_state state;
    int number_of_players;
    Client **list_of_players;
    Card *cards;
    int index_of_the_card;

} Game;


Game *create_games(int count_games, int count_players);
int add_player(Game *game,  Client *client, int players_in_game);
Game *get_game(Game *games, int count, int id_game);

void print_games(Game *games, int count);
static void print_game(Game *game);
void send_game_info(int sock_id, Game **list_of_games, int game_id, int count);

#endif //SP1_GAME_H
