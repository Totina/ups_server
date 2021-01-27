/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020
 *  Modul game_manager.c
 *
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "game_manager.h"
#include "game.h"
#include "cards.h"

/**
 * Inicializuje stav hry
 *
 * @param Game hra
 * @param client první klient ve hře
 * @param players_in_game počet hráčů
 *
 * @return EXIT_SUCCESS nebo EXIT_FAILURE
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

            printf("preparing game\n");
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
 * Start hry
 *
 * @param game - hra
 * @param players_in_game počet hráčů ve hře
 * @param mutex mutex
 */
void start_game(Game *game, int players_in_game, pthread_mutex_t mutex) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    if (game && players_in_game == game->number_of_players) {

        // change state to running
        game->state = GAME_STATE_IN_GAME;

        printf("starting game");

        // prepare cards
        //pthread_mutex_lock(&mutex);
        game->cards = prepare_cards();
        //pthread_mutex_unlock(&mutex);

        // message
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "game_started", END_CHAR);

        //pthread_mutex_lock(&mutex);
        // set client states to IN GAME, send message the game started
        for(int i = 0; i < game->number_of_players; i++) {
            game->list_of_players[i]->state = CLIENT_STATE_IN_GAME;
            send_message_to_client(game->list_of_players[i]->sock_id, server_message);
        }

        for(int i = 0; i < game->number_of_players; i++) {

            for(int j = 0; j < game->number_of_players; j++) {
                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", GAME_PREFIX, "player",
                         game->list_of_players[j]->name, END_CHAR);
                send_message_to_client(game->list_of_players[i]->sock_id, server_message);
            }
        }

        //pthread_mutex_unlock(&mutex);

        // deal cards
        //pthread_mutex_lock(&mutex);
        deal_starting_cards(game);
        //pthread_mutex_unlock(&mutex);

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
 * @param game hra
 * @return  0 nekdo chce harty, hra bude pokracovat
 *          1 nikdo nechce dalsi karty, hra se ukonci
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
* Ukončení hry a rozhodnutí o výherci.
 *
 * @param game hra
*/
void game_finished(Game *game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    game->state = GAME_STATE_FINISHED;

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
 * Odchod hráče ze hry
 *
 *
 * @param client hráč
 * @param game hra
 * @param players_in_game počet hráčů ve hře
 */
void client_left_the_game(Client *client, Game * game, int players_in_game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    if (client && game) {

        // message - player left
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", GAME_PREFIX, "player_left", client->name, END_CHAR);

        if(game->state == GAME_STATE_FINISHED) {
            printf("Player left after the game was finished\n");

            client->state = CLIENT_STATE_LOBBY;
            client->wants_another_card = 0;
            client->number_of_cards_in_hand = 0;
            client->game_id = -1;
            game->number_of_players--;

            if(game->number_of_players == 0) {
                game->state = GAME_STATE_WAITING;
            }

        }
        else if(game->state == GAME_STATE_IN_GAME || game->state == GAME_STATE_FULL) {
            printf("Player left while in game.\n");

            client->state = CLIENT_STATE_LOBBY;
            client->wants_another_card = 0;
            client->number_of_cards_in_hand = 0;
            client->game_id = -1;

            // send message to all other players that player left the game
            for (int i = 0; i < game->number_of_players; i++) {
                if (game->list_of_players[i]->sock_id != client->sock_id && game->list_of_players[i]->state != CLIENT_STATE_DISCONNECTED) {
                    send_message_to_client(game->list_of_players[i]->sock_id, server_message);
                }
            }

            // 1 or 2 players
            if(players_in_game < 3 || game->number_of_players < 3) {

                // player was alone
                if(game->number_of_players == 1) {
                    game->state = GAME_STATE_WAITING;
                }
                // the second player was disconnected
                else if(game->list_of_players[0]->state == CLIENT_STATE_DISCONNECTED ||
                game->list_of_players[1]->state == CLIENT_STATE_DISCONNECTED) {
                    game->state = GAME_STATE_WAITING;
                    //game->number_of_players--;
                }
                // the second player is still there
                else {
                    game->state = GAME_STATE_FINISHED;
                }

                end_game(game);
                //printf("Ending the game\n");
                game->number_of_players--;


            }
            else {
                // more than 2 players can continue playing

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
                game->number_of_players--;
            }

        }
        else if(game->state == GAME_STATE_WAITING) {
            printf("Player left the game waiting for players.\n");

            // send message to all other players that player left the game
            for (int i = 0; i < game->number_of_players; i++) {
                if (game->list_of_players[i]->sock_id != client->sock_id) {
                    send_message_to_client(game->list_of_players[i]->sock_id, server_message);
                }
            }

            client->state = CLIENT_STATE_LOBBY;
            client->wants_another_card = 0;
            client->number_of_cards_in_hand = 0;
            client->game_id = -1;

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

            game->number_of_players--;

        }


    } else {
        printf("Error in client left the game. Wrong parameters \n");
    }

}

/**
 * Ukončí a znova inicizalizuje hru pro další použití.
 *
 * @param client hráč
 * @param list_of_all_clients lsit všech klientu na serveru
 */
void end_game(Game *game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    printf("Ending the game\n");

    // clients
    for (int i = 0; i < game->number_of_players; i++) {
        //if (game->list_of_players[i]->state != CLIENT_STATE_DISCONNECTED) {
            //game->list_of_players[i]->state = CLIENT_STATE_LOBBY;
            //game->list_of_players[i]->game_id = -1;
            game->list_of_players[i]->number_of_cards_in_hand = 0;
            game->list_of_players[i]->wants_another_card = 0;

        if(game->list_of_players[i]->state == CLIENT_STATE_DISCONNECTED) {
            game->list_of_players[i]->state = CLIENT_STATE_LOBBY;
            game->list_of_players[i]->game_id = -1;
            game->number_of_players--;
        }

            game->list_of_players[i] = NULL;
        //}


    }

    free(game->cards);
    //free_cards();

    //game->number_of_players = 0;
    game->index_of_the_card = 0;
    //game->state = GAME_STATE_WAITING;

    //free_just_client_in_list(client->my_game->list_of_players);

    printf("Game ended \n");

}

/**
 * Set client as disconnected and adjust games accordingly.
 * Client leaves in lobby - reconnects to lobby
 * Client leaves after the game was finished - reconnects to the lobby
 * Client leaves waiting for players - reconnects to the lobby
 *
 * Client leaves when the game is running - reconnects to the game
 *
 * @param list
 */
void set_disconnected(Client *client, Game *game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    // message - player left
    snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", GAME_PREFIX, "player_left", client->name, END_CHAR);

    if (client) {
        printf("Setting disconnected to the client: %s\n", client->name);

        // client in the lobby when disconnected
        if(client->state == CLIENT_STATE_LOBBY) {
            printf("Player disconnected from the lobby.\n");
            client->before_disconnect_state = client->state;
            client->state = CLIENT_STATE_DISCONNECTED;
            return;
        }

        // if game exists
        if(game) {
            printf("Player disconnected from the game. State: %d\n", game->state);

            // game was finished
            if(game->state == GAME_STATE_FINISHED) {
                printf("Player disconnected after the game was finished.\n");

                client->before_disconnect_state = CLIENT_STATE_LOBBY;
                client->state = CLIENT_STATE_DISCONNECTED;
                client->wants_another_card = 0;
                client->number_of_cards_in_hand = 0;
                client->game_id = -1;
                game->number_of_players--;

                if(game->number_of_players == 0) {
                    game->state = GAME_STATE_WAITING;
                }
            }
            // game was waiting for players
            else if(game->state == GAME_STATE_WAITING) {
                printf("Player disconnected from the game waiting for players.\n");

                // send message to all other players that player left the game
                for (int i = 0; i < game->number_of_players; i++) {
                    if (game->list_of_players[i]->sock_id != client->sock_id && game->list_of_players[i]->state != CLIENT_STATE_DISCONNECTED) {
                        send_message_to_client(game->list_of_players[i]->sock_id, server_message);
                    }
                }

                client->before_disconnect_state = CLIENT_STATE_LOBBY;
                client->state = CLIENT_STATE_DISCONNECTED;
                client->wants_another_card = 0;
                client->number_of_cards_in_hand = 0;
                client->game_id = -1;

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

                game->number_of_players--;

            }
            // game is running
            else if(game->state == GAME_STATE_IN_GAME) {
                printf("Player disconnected from the running game.\n");

                // message - player disconnected
                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", GAME_PREFIX, "player_disconnected", client->name, END_CHAR);

                // send message to all the other players that player disconnected
                for (int i = 0; i < game->number_of_players; i++) {
                    if (game->list_of_players[i]->sock_id != client->sock_id && game->list_of_players[i]->state != CLIENT_STATE_DISCONNECTED) {
                        send_message_to_client(game->list_of_players[i]->sock_id, server_message);
                    }
                }

                client->before_disconnect_state = CLIENT_STATE_IN_GAME;
                client->state = CLIENT_STATE_DISCONNECTED;

            }

        }
        else {
            printf("Error: setting disconnected, game is NULL\n");
        }


    } else {
        printf("Error: setting disconnected, client is NULL\n");
    }

}

/**
 *
 *
 * @param list
 * @param client
 * @param message
 * @param number_of_games
 * @return
 */
int is_there_disconnected_client(Client_list *list, Client *client, Game **list_of_games, Message_in *message, int number_of_games,
        int players_in_game) {

    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    Client *tmp;

    // name
    char * name;
    name = message->arguments[0];
    int length = strlen(name);

    // get disconnected client with the same name
    tmp = get_client_by_name(list, name);
    if(tmp != NULL && tmp->state == CLIENT_STATE_DISCONNECTED) {
        printf("Found disconnected client with the same name: %s\n", tmp->name);

        // set name and add client to the list
        strcpy(client->name, name);
        add_client(list, client);

        // logged in message
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", LOGIN_PREFIX, "logged_in", client->name, END_CHAR);
        send_message_to_client(client->sock_id, server_message);

        // sending message about number of games
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %d%c", LOBBY_PREFIX,
                 "number_of_games", number_of_games, END_CHAR);
        send_message_to_client(client->sock_id, server_message);

        // client was in lobby
        if(tmp->before_disconnect_state == CLIENT_STATE_LOBBY) {
            printf("Client was in lobby when he disconnected\n");
            client->state = CLIENT_STATE_LOBBY;
        }
        // client was in game
        else if(tmp->game_id != -1 && tmp->before_disconnect_state == CLIENT_STATE_IN_GAME){
            printf("Client was playing when he disconnected\n");

            // get game he disconnected from
            Game *game = get_game(*list_of_games, number_of_games, tmp->game_id);


            // set state and game_id
            client->game_id = tmp->game_id;
            client->state = CLIENT_STATE_IN_GAME;

            // message entered
            snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %d %d%c", LOBBY_PREFIX, "entered", client->game_id, players_in_game, END_CHAR);
            send_message_to_client(client->sock_id, server_message);

            // message started
            snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", GAME_PREFIX, "game_started", END_CHAR);
            send_message_to_client(client->sock_id, server_message);

            // set cards
            client->number_of_cards_in_hand = tmp->number_of_cards_in_hand;
            client->wants_another_card = tmp->wants_another_card;

            for(int i = 0; i < client->number_of_cards_in_hand; i++) {
                client->cards[i] = tmp->cards[i];
            }

            // add player to the game
            for(int i = 0; i < game->number_of_players; i++) {
                printf("Looking to add player\n");
                if(strcmp(game->list_of_players[i]->name, name) == 0) {
                    printf("found place for the player\n");
                    game->list_of_players[i] = client;
                }
            }

            // message players
            for(int j = 0; j < game->number_of_players; j++) {
                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", GAME_PREFIX, "player",
                         game->list_of_players[j]->name, END_CHAR);
                send_message_to_client(client->sock_id, server_message);
            }

            // message cards
            for(int i = 0; i < client->number_of_cards_in_hand; i++) {
                // message - G card name value pattern
                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s %d %d%c", GAME_PREFIX, "card", client->cards[i].name,
                         client->cards[i].value, client->cards[i].pattern, END_CHAR);
                send_message_to_client(client->sock_id, server_message);
            }

            // message for others, that he is back
            snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", GAME_PREFIX, "player_reconnected",
                     client->name, END_CHAR);

            for (int i = 0; i < game->number_of_players; i++) {
                if (game->list_of_players[i]->sock_id != client->sock_id && game->list_of_players[i]->state != CLIENT_STATE_DISCONNECTED) {
                    send_message_to_client(game->list_of_players[i]->sock_id, server_message);
                }
            }

        }


    }
    else {
        return EXIT_FAILURE;
    }





    //free(tmp);
    close(tmp->sock_id);
    remove_client(list, tmp);

    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    return EXIT_SUCCESS;
}