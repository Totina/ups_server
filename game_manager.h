//
// Created by terez on 1/16/2021.
//

#ifndef SP1_GAME_MANAGER_H
#define SP1_GAME_MANAGER_H

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
void client_left_the_game(Client *client, Game * game, Client_list *list_of_clients, Game *list_of_games, int players_in_game);
void end_game(Client *client, Game * game, Client_list *list_of_all_clients);


#endif //SP1_GAME_MANAGER_H
