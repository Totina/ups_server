//
// Created by terez on 1/16/2021.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "game_manager.h"
#include "game.h"
#include "cards.h"

/**
 * Funkce inicializuje stav hry/místnosti a pošle zprávu klientovi, který vstoupil do hry
 *
 * @param Game - místnoti/hry
 * @param client první klient ve hře
 * @param list_of_clients list všech klientů připojených do hry
 * @return EXIT_SUCCESS pokud sepdoaří inicializovat hru, naopak EXIT_FAILURE
 */
int prepare_game(Game *game, Client *client, int players_in_game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    if (game && client) {
        if (game->state == GAME_STATE_NONE) {       // game is not prepared
            // set states to waiting
            game->state = GAME_STATE_WAITING;
            client->state = CLIENT_STATE_WAITING;
            client->game_id = game->id_game;

            // add player to game
            add_player(game, client, players_in_game);

            return EXIT_SUCCESS;

        } else {
            printf("Error preparing game. Game is not in the correct state.)\n");
            return EXIT_FAILURE;
        }
    } else {
        printf("Error preparing game. Wrong parameters.)\n");
        return EXIT_FAILURE;
    }
}

/**
 * Funkce, která odstrtuje hru.
 *  1) rozdají se karty hráčům
 *  2) hráčům je poslaná zpráva že můžou hrát - START_GAME
 *
 * @param client - jend áse o klienta, kter ýje do hry přidán jako poslední
 * @param list_of_clients list všech klientů na serveru
 * @param mutex mutex pro zamknutí bloku
 */
void start_game(Game *game, int players_in_game, pthread_mutex_t mutex) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    if (game && players_in_game == game->number_of_players) {
        //pthread_mutex_lock(&mutex);
        // change state to running
        game->state = GAME_STATE_IN_GAME;

        // prepare cards
        game->cards = prepare_cards();


        // message
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "game_started", END_CHAR);

        // set client states to IN GAME, send message the game started
        for(int i = 0; i < game->number_of_players; i++) {
            game->list_of_players[i]->state = CLIENT_STATE_IN_GAME;
            send_message_to_client(game->list_of_players[i]->sock_id, server_message);
        }
        //pthread_mutex_unlock(&mutex);

        // deal cards
        deal_starting_cards(game);

        memset(server_message, 0, MAX_LENGTH_MESSAGE);
    } else {
        printf("ERROR starting game. Wrong parameters.)\n");
    }
}

/**
 * Rozdá každému hráči dvě karty a pošle mu zprávu s jejich hodnotami.
 *
 */
void deal_starting_cards(Game * game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    // deal two cards to every player and send them in the message
    for(int i = 0; i < game->number_of_players; i++) {
        game->list_of_players[i]->cards[0] = game->cards[game->index_of_the_card];
        game->index_of_the_card++;
        game->list_of_players[i]->number_of_cards_in_hand++;

        // message - G card name value pattern
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s %d %d%c", GAME_PREFIX, "card", game->list_of_players[i]->cards[0].name,
                 game->list_of_players[i]->cards[0].value, game->list_of_players[i]->cards[0].pattern, END_CHAR);
        send_message_to_client(game->list_of_players[i]->sock_id, server_message);

        game->list_of_players[i]->cards[1] = game->cards[game->index_of_the_card];
        game->index_of_the_card++;
        game->list_of_players[i]->number_of_cards_in_hand++;

        // message - G card name value pattern
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s %d %d%c", GAME_PREFIX, "card", game->list_of_players[i]->cards[1].name,
                 game->list_of_players[i]->cards[1].value, game->list_of_players[i]->cards[1].pattern, END_CHAR);
        send_message_to_client(game->list_of_players[i]->sock_id, server_message);
    }

    memset(server_message, 0, MAX_LENGTH_MESSAGE);
}

/**
 * Dá hráči kartu z vrchu balíčku.
 *
 */
int gimme_card(Game * game, Client *client) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    if (game && client) {
        if(game->index_of_the_card < NUMBER_OF_CARDS) {

            if(client->wants_another_card == 1) {
                printf("Client can't get another card. Either is hand is full, or he declined card earlier.\n");
                return EXIT_FAILURE;
            }

            // adding card to the client
            if(client->number_of_cards_in_hand < MAX_CARDS_IN_HAND) {
                client->cards[client->number_of_cards_in_hand] = game->cards[game->index_of_the_card];
                game->index_of_the_card++;

                printf("Sending card.\n");

                // message - G card name value pattern
                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s %d %d%c", GAME_PREFIX, "card",
                         client->cards[client->number_of_cards_in_hand].name,
                         client->cards[client->number_of_cards_in_hand].value,
                         client->cards[client->number_of_cards_in_hand].pattern, END_CHAR);
                send_message_to_client(client->sock_id, server_message);

                client->number_of_cards_in_hand++;

            } else {
                printf("Full hand of cards.\n");
                client->wants_another_card = 1;
                // message to the client
                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "full_hand_of_cards", END_CHAR);
                send_message_to_client(client->sock_id, server_message);
                return EXIT_FAILURE;

            }
        }
        else {
            printf("No cards left.\n");
            // message to the client
            snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "no_cards_left", END_CHAR);
            send_message_to_client(client->sock_id, server_message);
            return EXIT_FAILURE;
        }
    }
    else {
        printf("Error dealing card. Wrong parameters.\n");
        return EXIT_FAILURE;
    }
}

/**
 * Zkontroluje, zda žádný hráč ve hře již nechce další kartu.
 *
 * @return  0 nekdo chce harty, hra bude pokracovat
 *          1 nikdo neche dalsi karty, hra se ukonci
 */
int no_more_cards_requested(Game *game) {
    for(int i = 0; i < game->number_of_players; i++) {
        if(game->list_of_players[i]->wants_another_card == 0) {
            return 0;
        }
    }
    return 1;
}

/**
*
*/
void game_finished(Game *game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    // winner
    int best_points = 0;
    int index_of_the_winner = -1;

    // tie solution
    int tie = 0;
    int first_tie_index = -1;
    int second_tie_index = -1;

    // count points
    for(int i = 0; i < game->number_of_players; i++) {
        int total = 0;

        for(int j = 0; j < game->list_of_players[i]->number_of_cards_in_hand; j++) {
            total += game->list_of_players[i]->cards[j].value;
        }

        if (total > best_points && total <= 21) {
            best_points = total;
            index_of_the_winner = i;
            tie = 0;        // more points than the tied players
        }
        else if(total == best_points) {
            tie = 1;
            first_tie_index = i;
            second_tie_index = index_of_the_winner;
        }
    }

    // messages to players
    if(index_of_the_winner == -1) {
        // all losers
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "lose", END_CHAR);
        for(int i = 0; i < game->number_of_players; i++) {
            send_message_to_client(game->list_of_players[i]->sock_id, server_message);
        }
    }
    else if(tie == 1) {
        // winners
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "win", END_CHAR);
        send_message_to_client(game->list_of_players[first_tie_index]->sock_id, server_message);
        send_message_to_client(game->list_of_players[second_tie_index]->sock_id, server_message);

        // losers
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "lose", END_CHAR);
        for(int i = 0; i < game->number_of_players; i++) {
            if(i != first_tie_index || i != second_tie_index) {
                send_message_to_client(game->list_of_players[i]->sock_id, server_message);
            }
        }
    }
    else {
        // winner
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "win", END_CHAR);
        send_message_to_client(game->list_of_players[index_of_the_winner]->sock_id, server_message);

        // losers
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "lose", END_CHAR);
        for(int i = 0; i < game->number_of_players; i++) {
            if(i != index_of_the_winner) {
                send_message_to_client(game->list_of_players[i]->sock_id, server_message);
            }
        }
    }
}

/**
 * Funkce, která ošetří odchod hráče ze hry.
 * Dělí se na dva stavy:
 *  1) odchod z probíhající hry -> konec hry
 *  2) odchoz ze hry čekající na dalšího hráče
 *
 * @param client odcházející hráč
 * @param list_of_clients lsit všech klientů
 * @param list_of_games list všech her
 */
void client_left_the_game(Client *client, Game * game, Client_list *list_of_clients, Game *list_of_games, int players_in_game) {

    if (client && list_of_clients && list_of_games) {
        if (client->state == CLIENT_STATE_IN_GAME) {
            //HRA BEZI A KLIENT ODCHAZI
            // resetovani hry pro novou hru - pro nove hrace
            if(players_in_game < 3) {
                end_game(client, game, list_of_clients);
            }


        } else if (client->state == CLIENT_STATE_WAITING) {
            //HRA BYLA INICIALIZOVANA, CEKA SE NA HRACE A KLIENT ODESEL
            client->state = CLIENT_STATE_LOBBY;

            for (int i = 0; i < game->number_of_players; i++) {
                if (game->list_of_players[i]->name == client->name) {
                    game->list_of_players[i] = NULL;

                    for(int j = 0; j < game->number_of_players - i - 1; j++) {
                        if(game->list_of_players[i+1]) {
                            game->list_of_players[i] = game->list_of_players[i+1];
                        }
                    }
                }
            }

        }

    } else {
        printf("Parameters are NULL - (void client_leave_from_game(Client_s *client, LinkedList_clients_s *list_of_clients, LinkedList_game_rooms_s *list_of_games))\n");
    }

}

/**
 * Funkce, která ukončí a znova inicizalizuje hru. Aby do hry mohl přijít jiní hráči
 *
 * @param client Klient ze hry
 * @param list_of_all_clients lsit všech klientu na serveru
 */
void end_game(Client *client, Game * game, Client_list *list_of_all_clients) {
    //nastavit klientum novy stav a hru na NULL

    for (int i = 0; i < game->number_of_players; i++) {
        if (game->list_of_players[i]->state != CLIENT_STATE_DISCONNECTED) {
            game->list_of_players[i]->state = CLIENT_STATE_LOBBY;
            game->list_of_players[i]->game_id = -1;
            game->list_of_players[i]->number_of_cards_in_hand = 0;
            game->list_of_players[i]->wants_another_card = 0;
            game->list_of_players[i] = NULL;
        }
    }

    printf("ending the game \n");

    free(game->cards);
    //free_cards();

    game->number_of_players = 0;
    game->index_of_the_card = 0;
    game->state = GAME_STATE_WAITING;

    //free_just_client_in_list(client->my_game->list_of_players);

}