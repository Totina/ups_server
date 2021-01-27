/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020
 *
 */

#ifndef SP1_GAME_MANAGER_H
#define SP1_GAME_MANAGER_H

#include <unistd.h>
#include "game.h"
#include "client.h"

#define NUMBER_OF_CARDS 32
#define MAX_CARDS_IN_HAND 12

int prepare_game(Game *game, Client *client, int players_in_game);
void start_game(Game *game, int players_in_game, pthread_mutex_t mutex);
void deal_starting_cards(Game * game);
int gimme_card(Game * game, Client *client);
int no_more_cards_requested(Game *game);
void game_finished(Game *game);
void client_left_the_game(Client *client, Game *game, int players_in_game);
void end_game(Game *game);
void set_disconnected(Client *client, Game *game);
int is_there_disconnected_client(Client_list *list, Client *client, Game **list_of_games, Message_in *message, int number_of_games,
                                 int players_in_game);
#endif //SP1_GAME_MANAGER_H
