/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020
 *  Modul server.c
 *
 *
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "client.h"
#include "header.h"
#include "game.h"
#include "message_in.h"
#include "game_manager.h"

// default max values
int const MAX_PLAYERS = 5;
int const MAX_GAMES = 10;

// number of games
int number_of_games = 5;        // number of open game rooms
int max_players_in_game = 3;    // max number of players in one game room

// Client list
Client_list *list_of_clients;
Game * list_of_games;

// mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// funkce
void *serve_request(void *new_client);
void print_server();
void leave_the_game(Client *client, Game *game);

int main(int argc, char *argv[]){

    // default settings
	int port = 40000;
	int max_players_overall = 15;   // max number of players in game overall (all rooms together)

	// struct for sockets
	struct sockaddr_in my_addr, peer_addr;

	memset(&my_addr, 0, sizeof(struct sockaddr_in));

	// creates my_addr
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	// sockets
    int server_socket;
    int client_socket;
    int return_value;

    // accept connection stuff
    int *th_socket;
    socklen_t	remote_addr_len;        // socklen_t = type definition for length and size values used by socket related parameters, platform independant
    pthread_t thread_id;

	// arguments
	if(argc == 1) {     // no arguments, default settings
        arg_info();
        printf("IP is not set.\n");
        printf("Port set to %d\n", port);
        printf("Number of games set to %d\n", number_of_games);
        printf("Number of players set to %d\n", number_of_games);
    }
	else if(argc % 2 == 0) {        // even number of arguments
        printf("Error: Wrong number of parameters\n");
        return EXIT_FAILURE;
    }
	else {                          // odd number of arguments
        for (int i = 1; i < argc; i += 2) {
            if(strcmp(argv[i], "-address") == 0 || strcmp(argv[i], "-a") == 0) {
                my_addr.sin_addr.s_addr = inet_addr(argv[i+1]);
                printf("Address set to %s\n", argv[i+1]);
            }
            else if(strcmp(argv[i], "-port") == 0 || strcmp(argv[i], "-p") == 0) {
                my_addr.sin_port = htons(atoi(argv[i+1]));
                printf("Port set to %d\n", atoi(argv[i+1]));
            }
            else if(strcmp(argv[i], "-games") == 0 || strcmp(argv[i], "-g") == 0) {
                int tmp = atoi(argv[i+1]);
                if(tmp > MAX_GAMES) {
                    number_of_games = MAX_GAMES;
                    printf("Max number of games is %d\n", MAX_GAMES);
                    printf("Number of games set to %d\n", MAX_GAMES);
                }
                else {
                    number_of_games = atoi(argv[i+1]);
                    printf("Number of games set to %d\n", atoi(argv[i+1]));
                }
            }
            else if(strcmp(argv[i], "-players") == 0 || strcmp(argv[i], "-pl") == 0) {
                int tmp = atoi(argv[i+1]);
                if(tmp > MAX_PLAYERS) {
                    max_players_in_game = MAX_PLAYERS;
                    printf("Max number of players is %d\n", MAX_PLAYERS);
                    printf("Number of players set to %d\n", MAX_PLAYERS);
                }
                else {
                    max_players_in_game = atoi(argv[i+1]);
                    printf("Number of players set to %d\n", atoi(argv[i+1]));
                }
            }
            else {
                printf("Error: Wrong parameters\n");
                return EXIT_FAILURE;
            }
        }
    }

	// adjusting number of overall players
    max_players_overall = number_of_games * max_players_in_game;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
        printf("Error creating socket");
        return EXIT_FAILURE;
	}
	else {
        printf("Socket created\n");
	}

    // Bind socket
    return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));

    if (return_value == 0) {
        printf("Bind - OK\n");
    }
    else {
        printf("Bind - ERR\n");
        return EXIT_FAILURE;
    }

    // Listen
    return_value = listen(server_socket, 5);
    if (return_value == 0) {
        printf("Listen - OK\n");
    } else {
        printf("Listen - ERR\n");
        return EXIT_FAILURE;
    }

    // Create list of clients
    list_of_clients = create_client_list();
    if (!list_of_clients) {
        printf("Error: wasn't able to create list for clients");
        return EXIT_FAILURE;
    }

    // TEST Create clients
    /*Client *new_client = create_client("Pepa", "128.9.8.2", 90);
    add_client(list_of_clients, new_client);

    Client *new_client2 = create_client("Anna", "122.9.8.2", 20);
    add_client(list_of_clients, new_client2);

    Client *new_client3 = create_client("Bum", "120.9.8.2", 10);
    add_client(list_of_clients, new_client3);*/

    // TEST Print list of clients
    //print_client_list(list_of_clients);

    // Create list of games
    list_of_games = create_games(number_of_games, max_players_in_game);
    //print_games(list_of_games, number_of_games);

    // TEST
    /*Game *game1 = get_game(list_of_games, number_of_games, 0);
    game1->cards = prepare_cards();
    print_cards(game1->cards);*/


    /*add_player(game1, new_client);
    add_player(game1, new_client3);

    print_games(list_of_games, number_of_games);
    print_client_list(list_of_clients);*/

    // tests
    /*Message_in *message1;
    char client_message1[MAX_LENGTH_MESSAGE];
    strcpy(client_message1, "G card;");

    message1 = parse_in_message(client_message1);
    print_message(message1);*/


    // Waiting for connections, ready to accept new client sockets
    printf("\nWaiting for connections...\n");

    // Accepting client sockets
    while(1){
        client_socket = accept(server_socket, (struct sockaddr *)&peer_addr, (socklen_t * ) &remote_addr_len);

        if (client_socket > 0 ) {
            // creating and allocating space for client socket
            th_socket = malloc(sizeof(int));
            *th_socket = client_socket;

            // create ip address for new client
            struct sockaddr_in *pV4Addr = (struct sockaddr_in *) &peer_addr;
            struct in_addr ipAddr = pV4Addr->sin_addr;

            // ip address - INET_ADDRSTRLEN = 16
            char ip_address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ipAddr, ip_address, INET_ADDRSTRLEN);

            // create new client
            Client *new_client = create_client("new_client", ip_address, client_socket);

            pthread_create(&thread_id, NULL, (void *)&serve_request, (void *)new_client);
        } else {
            printf("Error: accept failure\n");
            return EXIT_FAILURE;
        }
    }

	return 0;
}

/**
 * Telo vlakna co obsluhuje prichozi spojeni
 *
 * */
void *serve_request(void *new_client){

    // size of the message
    int read_size;

    // incoming message
    Message_in *message;
    char client_message[MAX_LENGTH_MESSAGE];
    memset(client_message, 0, MAX_LENGTH_MESSAGE);

    // outgoing message
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    // pretypovani z netypoveho ukazate na ukazatel na Client
    Client *client = (Client *) new_client;
    printf("(Vlakno:) Huraaa nove spojeni\n");

    //Receive a message from client
    while ((read_size = recv(client->sock_id, client_message, MAX_LENGTH_MESSAGE, 0)) > 0) {
        //client_message[read_size] = '\0';
        printf("[RECEIVED]: %s\n", client_message);

        // parsovani zpravy
        message = parse_in_message(client_message);
        print_message(message);

        // if message not null, switch according to message prefix
        if(message) {
            switch (message->prefix) {
                /************************************* LOGIN ******************************************/
                case LOGIN_PREFIX:
                    printf("\n");

                    // setting a name
                    int result = set_name(list_of_clients, client, message);
                    if (result == EXIT_SUCCESS) {
                        // adding client to the list of clients
                        add_client(list_of_clients, client);

                        // sending message about number of games to client
                        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %d%c", LOBBY_PREFIX,
                                 "number_of_games", number_of_games, END_CHAR);
                        send_message_to_client(client->sock_id, server_message);

                        // change client state
                        client->state = CLIENT_STATE_LOBBY;
                        //print_server();

                        memset(server_message, 0, MAX_LENGTH_MESSAGE);
                    }
                    else {
                        printf("Error logging in.\n");
                        snprintf(server_message, MAX_LENGTH_MESSAGE, "ERROR_LOGGING_IN");
                        //error_out_fo_state_disconnect (client, server_message);
                        memset(server_message, 0, MAX_LENGTH_MESSAGE);
                    }
                    memset(client_message, 0, MAX_LENGTH_MESSAGE);
                    memset(server_message, 0, MAX_LENGTH_MESSAGE);
                    if (message) free_message(message);
                    break;


                    /************************************** LOBBY *************************************/
                case LOBBY_PREFIX:
                    if (client->state == CLIENT_STATE_LOBBY) {
                        if (message->number_of_arguments == 2 && strcmp(message->arguments[0], "game") == 0) {      // client zada info o game
                            int id_game = atoi(message->arguments[1]);

                            // find the correct game
                            Game *game = get_game(list_of_games, number_of_games, id_game);

                            // if game exists
                            if(game) {
                                // send info to client
                                send_game_info(client->sock_id, &list_of_games, game->id_game, number_of_games);
                            }
                            else {
                                printf("Error: game with this id doesn't exist \n");
                                snprintf(server_message, MAX_LENGTH_MESSAGE, "ERROR_CHOOSING_GAME ");
                                //error_out_fo_state_disconnect(client, message_for_send);
                                memset(server_message, 0, MAX_LENGTH_MESSAGE);
                            }

                        } else if (message->number_of_arguments == 2 && strcmp(message->arguments[0], "enter") == 0) {        // client chce vstoupit
                            int id_game = atoi(message->arguments[1]);
                            // find the correct game
                            Game *game = get_game(list_of_games, number_of_games, atoi(message->arguments[1]));
                            print_server();
                            // if game exists
                            if(game){
                                if(game->state == GAME_STATE_NONE) {
                                    // game exists, but is empty, prepare game
                                    pthread_mutex_lock(&mutex);
                                    prepare_game(game, client, max_players_in_game);
                                    pthread_mutex_unlock(&mutex);
                                }
                                else if (game->state == GAME_STATE_WAITING && game->number_of_players < max_players_in_game) {
                                    // game is waiting for players
                                    pthread_mutex_lock(&mutex);
                                    add_player(game, client, max_players_in_game);
                                    pthread_mutex_unlock(&mutex);
                                }

                                // is the game full?
                                if (game->state == GAME_STATE_FULL && game->number_of_players == max_players_in_game){
                                    // start game
                                    pthread_mutex_lock(&mutex);
                                    start_game(game, max_players_in_game, mutex);
                                    pthread_mutex_unlock(&mutex);
                                    printf("state: %d", client->state);

                                }
                            } else {
                                printf("Error: game with this id doesn't exist\n");
                                snprintf(server_message, MAX_LENGTH_MESSAGE, "ERROR_CHOOSING_GAME ");
                                //error_out_fo_state_disconnect(client, message_for_send);
                                memset(server_message, 0, MAX_LENGTH_MESSAGE);
                            }
                        }
                    }
                    else if (client->state == CLIENT_STATE_IN_GAME) {
                        Game *game = get_game(list_of_games, number_of_games, client->game_id);

                        if (message->number_of_arguments == 1 && strcmp(message->arguments[0], "card") == 0) {
                            // give player a card
                            pthread_mutex_lock(&mutex);
                            gimme_card(game, client);
                            pthread_mutex_unlock(&mutex);
                        }
                        else if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "no_thanks") == 0) {
                            // player doesn't want any more cards
                            client->wants_another_card = 1;

                            // did all players decline another card?
                            int game_stop_result;
                            game_stop_result = no_more_cards_requested(game);
                            if(game_stop_result == 1) {
                                // game is finished
                                game_finished(game);
                            }
                        }
                        else if (message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_the_game") == 0) {
                            printf("left");
                            leave_the_game(client, game);

                        }

                    }
                    else {
                        printf("Error: Out of state.\n");
                    }

                    memset(client_message, 0, MAX_LENGTH_MESSAGE);
                    memset(server_message, 0, MAX_LENGTH_MESSAGE);
                    if (message) free_message(message);
                    break;


                    /************************************** GAME *************************************/
                case GAME_PREFIX:
                    printf("xx");



                    break;

                case ERROR_PREFIX:
                    printf("err");

                    break;
            }


        }
        else {
            printf("Error: BAD FORM MESSAGE - name: %s, state: %d\n", client->name, client->state);
            //error_out_fo_state_disconnect(client, server_message);
            memset(server_message, 0, MAX_LENGTH_MESSAGE);
        }




        // makes the client message all 0
        memset(client_message, 0, MAX_LENGTH_MESSAGE);
    }






    // uvolnujeme pamet
    //free(arg);
    return 0;
}









/*
 * Print info into console how to write parameters for server
 */
void arg_info(){
	printf("How to start a server\n");
	printf("-address || -a [IPv4] : set listening address (valid IPv4), default INADDR_ANY\n");
	printf("-port || -p [number] : set listening port (1024-65535), default 40000\n");
    printf("-games || -g [number] : set number of games (1-20), default 5\n");
    printf("-players || -pl [number] : set max number of players is the game (2-5), default 3\n");
	printf("Example: ./server.exe -a 127.0.0.1 -p 10000 -g 5 -pl 3\n\n");
}

/**
 * Funkce, která vytiskne do konzole, na obrazovku stav her/klientů
 */
void print_server() {
    printf("\n-------------------------------GAME---------------------------------\n");
    printf("Games: \n");
    print_games(list_of_games, number_of_games);
    printf("Clients: \n");
    print_client_list(list_of_clients);
    printf("--------------------------------------------------------------------\n");

}


/**
 * Funkce ze avolá pokud klient odejde ze hry
 *
 * @param client Klient, který odeše lze hry
 */
void leave_the_game(Client *client, Game *game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    Client *current;
    if ((client->state == CLIENT_STATE_IN_GAME) == 0) {
        printf("Client (%s) left from running game. \n", client->name);

        send_message_to_client(client->sock_id, server_message);

        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", GAME_PREFIX, "player_left", client->name, END_CHAR);

        // send message to all other players that player left
        for (int i = 0; i < game->number_of_players; i++) {
            if (game->list_of_players[i]->sock_id != client->sock_id) {
                send_message_to_client(game->list_of_players[i]->sock_id, server_message);
            }

            client_left_the_game(client, game, list_of_clients, list_of_games, max_players_in_game);

        }

    } else if (client->state == CLIENT_STATE_WAITING) {
        printf("Client (%s) left from waiting game. \n", client->name);
        client_left_the_game(client, game, list_of_clients, list_of_games, max_players_in_game);

    }


}