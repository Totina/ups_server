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
#include <fcntl.h>
#include <unistd.h>

#include "client.h"
#include "game.h"
#include "message_in.h"
#include "game_manager.h"

// default max values
int const MAX_PLAYERS = 5;
int const MAX_GAMES = 10;

// default number of games and players
int number_of_games = 5;        // number of open game rooms
int max_players_in_game = 2;    // max number of players in one game room

// Client list
Client_list *list_of_clients;
Game * list_of_games;

// mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// file
FILE *fp;

// funkce
void *serve_request(void *new_client);
void *ping_thread(void *arg);
void print_server();
void leave_the_game(Client *client);
int isANumber(char *input);
void kickOut(Client *client);
void arg_info();

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

    // socket
    int *th_socket;
    socklen_t	remote_addr_len;        // socklen_t = type definition for length and size values used by socket related parameters, platform independant
    pthread_t thread_id;
    pthread_t thread_ping;

    // log file
    fp  = fopen ("data.log", "a+");
    fprintf(fp, "\nServer started\n");

	// arguments
	if(argc == 1) {     // no arguments, default settings
        arg_info();
        printf("IP is not set.\n");
        printf("Port set to %d\n", port);
        printf("Number of games set to %d\n", number_of_games);
        printf("Number of players set to %d\n", max_players_in_game);

        fprintf(fp, "IP is not set.\n");
        fprintf(fp,"Port set to %d\n", port);
        fprintf(fp,"Number of games set to %d\n", number_of_games);
        fprintf(fp,"Number of players set to %d\n", max_players_in_game);

    }
	else if(argc % 2 == 0) {        // even number of arguments
        printf("Error: Wrong number of parameters\n");
        fprintf(fp, "ERROR: Wrong number of parameters\n");
        return EXIT_FAILURE;
    }
	else {                          // odd number of arguments
        for (int i = 1; i < argc; i += 2) {
            if(strcmp(argv[i], "-address") == 0 || strcmp(argv[i], "-a") == 0) {
                my_addr.sin_addr.s_addr = inet_addr(argv[i+1]);
                printf("Address set to %s\n", argv[i+1]);
                fprintf(fp, "Address set to %s\n", argv[i+1]);
            }
            else if(strcmp(argv[i], "-port") == 0 || strcmp(argv[i], "-p") == 0) {

                if(isANumber(argv[i+1]) == 0) {
                    int tmp = atoi(argv[i+1]);

                    if(tmp > 1024 && tmp < 49151 ) {
                        my_addr.sin_port = htons(tmp);
                        printf("Port set to %d\n", tmp);
                        fprintf(fp, "Port set to %d\n", tmp);
                    }
                }
                else {
                    printf("Incorrect port. Port set to %d\n", port);
                }
            }
            else if(strcmp(argv[i], "-games") == 0 || strcmp(argv[i], "-g") == 0) {

                if(isANumber(argv[i+1]) == 0) {
                    int tmp = atoi(argv[i+1]);
                    if(tmp < MAX_GAMES) {
                        number_of_games = tmp;
                        printf("Number of games set to %d\n", tmp);
                        fprintf(fp, "Number of games set to %d\n", tmp);
                    }
                }
                else {
                    printf("Incorrect number of games\n");
                    printf("Number of games set to %d\n", number_of_games);
                    fprintf(fp, "Number of games set to %d\n", number_of_games);
                }
            }
            else if(strcmp(argv[i], "-players") == 0 || strcmp(argv[i], "-pl") == 0) {

                if(isANumber(argv[i+1]) == 0) {
                    int tmp = atoi(argv[i+1]);
                    if(tmp < MAX_PLAYERS) {
                        max_players_in_game = tmp;
                        printf("Number of players set to %d\n", tmp);
                        fprintf(fp, "Number of players set to %d\n", tmp);
                    }
                }
                else {
                    printf("Incorrect number of players\n");
                    printf("Number of players set to %d\n", max_players_in_game);
                    fprintf(fp, "Number of players set to %d\n", max_players_in_game);
                }
            }
            else {
                printf("Error: Wrong parameters\n");
                fprintf(fp, "ERROR: Wrong parameters\n");
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
        fprintf(fp, "ERROR: creating socket\n");
        return EXIT_FAILURE;
	}
	else {
        printf("Socket created\n");
        fprintf(fp, "Socket created\n");
	}

    // Bind socket
    return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));

    if (return_value == 0) {
        printf("Bind - OK\n");
        fprintf(fp, "Bind OK\n");
    }
    else {
        printf("Bind - ERR\n");
        fprintf(fp, "ERROR: bind\n");
        return EXIT_FAILURE;
    }

    // Listen
    return_value = listen(server_socket, 5);
    if (return_value == 0) {
        printf("Listen - OK\n");
        fprintf(fp, "Listen OK\n");
    } else {
        printf("Listen - ERR\n");
        fprintf(fp, "ERROR: Listen\n");
        return EXIT_FAILURE;
    }

    // Create list of clients
    list_of_clients = create_client_list();
    if (!list_of_clients) {
        printf("Error: could not create list of clients");
        fprintf(fp, "ERROR: could not create list of clients\n");
        return EXIT_FAILURE;
    }

    // Create list of games
    list_of_games = create_games(number_of_games, max_players_in_game);
    if(!list_of_games) {
        printf("Error: could not create list of games");
        fprintf(fp, "ERROR: could not create list of games\n");
        return EXIT_FAILURE;
    }

    fprintf(fp, "Server set up successful\n");
    fclose(fp);

    // ping thread
    int *param;
    param = malloc(sizeof(int));
    pthread_create(&thread_ping, NULL, (void *)&ping_thread, (void *)param);


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

            //pthread_join(thread_id, NULL);

        } else {
            printf("Error: Accept failure\n");
            return EXIT_FAILURE;
        }
    }

	return 0;
}


void *ping_thread(void *arg) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s%c", PING_PREFIX, "ping", END_CHAR);

    printf("Ping thread created\n");

    while(1) {
        printf("Starting pinging round;\n");
        // if there are any clients
        if(list_of_clients && list_of_clients->first) {
            // set ponged of all clients to 1 (no)
            set_ponged_to_default(list_of_clients);

            // send ping message to all clients
            ping_all_clients(list_of_clients);
        }
        else {
            printf("No clients to send ping to.\n");
        }

        sleep(5);

        if(list_of_clients && list_of_clients->first) {
            // set state disconnected to client who didn't ponged back
            Client *tmp = list_of_clients->first;
            while (tmp) {
                if (tmp->ponged == 1 && tmp->state != CLIENT_STATE_DISCONNECTED) {
                    printf("Client %s disconnected\n", tmp->name);
                    Game *game = get_game(list_of_games, number_of_games, tmp->game_id);
                    set_disconnected(tmp, game);
                }
                tmp = tmp->next;
            }



            print_server();
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
    printf("(Thread:) new connection\n");

    //Receive a message from client
    while ((read_size = recv(client->sock_id, client_message, MAX_LENGTH_MESSAGE, 0)) > 0) {


        // time out
        struct timeval tv;
        tv.tv_sec = 180;
        tv.tv_usec = 0;
        setsockopt(client->sock_id, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        setsockopt(client->sock_id, SOL_SOCKET, SO_REUSEADDR, (const char*)1, sizeof(int));


        //client_message[read_size] = '\0';
        printf("\n[RECEIVED]: %s\n", client_message);

        // parsovani zpravy
        message = parse_in_message(client_message);

        //print_message(message);

        // if message not null, switch according to message prefix
        if(message) {
            switch (message->prefix) {
                /************************************* LOGIN ******************************************/
                case LOGIN_PREFIX:
                    printf("\n");

                    // is player reconnecting?
                    int reconnecting;
                    reconnecting = is_there_disconnected_client(list_of_clients, client, &list_of_games, message,
                                                                number_of_games, max_players_in_game);

                    if(reconnecting == 0) {
                        printf("Reconnected client.");
                    }
                    else {
                        // STANDARD LOGIN
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

                            memset(server_message, 0, MAX_LENGTH_MESSAGE);
                        }
                        else {
                            printf("Error logging in.\n");
                            snprintf(server_message, MAX_LENGTH_MESSAGE, "ERROR_LOGGING_IN");
                            kickOut(client);
                            memset(server_message, 0, MAX_LENGTH_MESSAGE);
                        }
                    }

                    memset(client_message, 0, MAX_LENGTH_MESSAGE);
                    memset(server_message, 0, MAX_LENGTH_MESSAGE);

                    if (message) {
                        free_message(message);
                    }
                    break;

                    /************************************** LOBBY *************************************/
                case LOBBY_PREFIX:
                    printf("\nlobby\n");
                    printf("Client state: %d", client->state);
                    if (client->state == CLIENT_STATE_LOBBY) {
                        if (message->number_of_arguments == 2 && strcmp(message->arguments[0], "game") == 0) {      // client zada info o game
                            if(isANumber(message->arguments[1]) != 0) {
                                printf("Error: The game ID has to be a number.");
                                kickOut(client);
                                break;
                            }

                            int id_game = atoi(message->arguments[1]);

                            // find the correct game
                            Game *game = get_game(list_of_games, number_of_games, id_game);

                            // if game exists
                            if(game) {
                                // send info to client
                                pthread_mutex_lock(&mutex);
                                send_game_info(client->sock_id, &list_of_games, game->id_game, number_of_games);
                                pthread_mutex_unlock(&mutex);
                            }
                            else {
                                printf("Error: game with this id doesn't exist \n");
                                snprintf(server_message, MAX_LENGTH_MESSAGE, "ERROR_CHOOSING_GAME ");
                                kickOut(client);
                                memset(server_message, 0, MAX_LENGTH_MESSAGE);
                            }

                        } else if (message->number_of_arguments == 2 && strcmp(message->arguments[0], "enter") == 0) {        // client chce vstoupit
                            if(isANumber(message->arguments[1]) != 0) {
                                printf("Error: The game ID has to be a number.");
                                kickOut(client);
                                break;
                            }

                            int id_game = atoi(message->arguments[1]);
                            // find the correct game
                            Game *game = get_game(list_of_games, number_of_games, id_game);
                            //print_server();
                            // if game exists
                            if(game){
                                if(game->state == GAME_STATE_NONE) {
                                    // game exists, but is empty, prepare game
                                    pthread_mutex_lock(&mutex);
                                    prepare_game(game, client, max_players_in_game);
                                    pthread_mutex_unlock(&mutex);
                                    printf("game prepared\n");
                                }
                                else if (game->state == GAME_STATE_WAITING && game->number_of_players < max_players_in_game) {
                                    printf("waiting\n");
                                    // game is waiting for players
                                    pthread_mutex_lock(&mutex);
                                    add_player(game, client, max_players_in_game);
                                    pthread_mutex_unlock(&mutex);
                                }
                                else if(game->state == GAME_STATE_FULL || game->state == GAME_STATE_IN_GAME || game->state == GAME_STATE_FINISHED){
                                    printf("Can't enter\n");
                                    snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %d%c", LOBBY_PREFIX, "cant_enter", game->id_game, END_CHAR);
                                    send_message_to_client(client->sock_id, server_message);
                                }

                                // is the game full?
                                if (game->state == GAME_STATE_FULL && game->number_of_players == max_players_in_game){
                                    printf("full\n");
                                    // start game
                                    pthread_mutex_lock(&mutex);
                                    start_game(game, max_players_in_game, mutex);
                                    pthread_mutex_unlock(&mutex);

                                }
                            } else {
                                printf("Error: game with this id doesn't exist\n");
                                snprintf(server_message, MAX_LENGTH_MESSAGE, "ERROR_CHOOSING_GAME ");
                                kickOut(client);
                                memset(server_message, 0, MAX_LENGTH_MESSAGE);
                            }
                        }
                        else if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_the_game") == 0) {
                            printf("client left the game\n");
                            leave_the_game(client);
                        }
                        else {
                            printf("Error: Incorrect message.\n");
                            kickOut(client);
                        }
                    }
                    else {
                        printf("Error: Out of state.\n");
                        kickOut(client);
                    }

                    memset(client_message, 0, MAX_LENGTH_MESSAGE);
                    memset(server_message, 0, MAX_LENGTH_MESSAGE);
                    if (message) {
                        free_message(message);
                    }
                    break;


                    /************************************** GAME *************************************/
                case GAME_PREFIX:
                    printf("\ngame\n");
                    printf("state: %d\n", client->state);

                    if (client->state == CLIENT_STATE_IN_GAME) {

                        Game *game = get_game(list_of_games, number_of_games, client->game_id);

                        // in game
                        if(game->state == GAME_STATE_IN_GAME) {

                            if (message->number_of_arguments == 1 && strcmp(message->arguments[0], "card") == 0) {
                                // give player a card
                                printf("card \n");
                                gimme_card(game, client);
                            }
                            else if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "no_thanks") == 0) {
                                printf("no thanks\n");
                                // player doesn't want any more cards
                                client->wants_another_card = 1;

                                // did all players decline another card?
                                int game_stop_result;
                                game_stop_result = no_more_cards_requested(game);
                                if(game_stop_result == 1) {
                                    // game is finished
                                    pthread_mutex_lock(&mutex);
                                    game_finished(game);
                                    end_game(game);
                                    pthread_mutex_unlock(&mutex);
                                }
                            }
                            else if (message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_to_the_lobby") == 0) {
                                // player left before the game ended
                                printf("client left before the game was finished\n");
                                pthread_mutex_lock(&mutex);
                                client_left_the_game(client, game, max_players_in_game);
                                pthread_mutex_unlock(&mutex);
                            }
                            else if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_the_game") == 0) {
                                printf("client left the game\n");
                                pthread_mutex_lock(&mutex);
                                client_left_the_game(client, game, max_players_in_game);
                                leave_the_game(client);
                                pthread_mutex_unlock(&mutex);
                            }
                            else {
                                printf("Error: Incorrect message.\n");
                                kickOut(client);
                            }
                        }
                        // waiting or full
                        else if(game->state == GAME_STATE_WAITING || game->state == GAME_STATE_FULL) {
                            if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_to_the_lobby") == 0) {
                                // player left before the game ended
                                printf("client left before the game was finished\n");
                                pthread_mutex_lock(&mutex);
                                client_left_the_game(client, game, max_players_in_game);
                                pthread_mutex_unlock(&mutex);
                            }
                            else if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_the_game") == 0) {
                                printf("client left the game\n");
                                pthread_mutex_lock(&mutex);
                                client_left_the_game(client, game, max_players_in_game);
                                leave_the_game(client);
                                pthread_mutex_unlock(&mutex);
                            }
                        }
                        // finished
                        else if(game->state == GAME_STATE_FINISHED) {
                            if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_to_the_lobby") == 0) {                            // player left after game ended
                                printf("client left after the fame was finished\n");
                                pthread_mutex_lock(&mutex);
                                client_left_the_game(client, game, max_players_in_game);
                                pthread_mutex_unlock(&mutex);
                            }
                            else if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_the_game") == 0) {
                                printf("client left the game\n");
                                pthread_mutex_lock(&mutex);
                                client_left_the_game(client, game, max_players_in_game);
                                leave_the_game(client);
                                pthread_mutex_unlock(&mutex);
                            }
                        }
                        else {
                            printf("Out of state: can't play when the game is not running.\n");
                        }
                    }
                    else if (message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_to_the_lobby") == 0) {
                        Game *game = get_game(list_of_games, number_of_games, client->game_id);

                        printf("client left to the lobby\n");
                        printf("client name: %s\n", client->name);
                        printf("game id: %d\n", game->id_game);

                        if(game->state == GAME_STATE_IN_GAME || game->state == GAME_STATE_WAITING || game->state == GAME_STATE_FULL) {
                            // player left before the game ended
                            printf("client left before the game was finished\n");
                            pthread_mutex_lock(&mutex);
                            client_left_the_game(client, game, max_players_in_game);
                            pthread_mutex_unlock(&mutex);
                        }
                        else if(game->state == GAME_STATE_FINISHED) {
                            // player left after game ended
                            printf("client left after the fame was finished\n");
                            pthread_mutex_lock(&mutex);
                            client_left_the_game(client, game, max_players_in_game);
                            pthread_mutex_unlock(&mutex);
                        }

                    }
                    else if(message->number_of_arguments == 1 && strcmp(message->arguments[0], "left_the_game") == 0) {
                        Game *game = get_game(list_of_games, number_of_games, client->game_id);
                        printf("client left the game\n");
                        pthread_mutex_lock(&mutex);
                        client_left_the_game(client, game, max_players_in_game);
                        leave_the_game(client);
                        pthread_mutex_unlock(&mutex);
                    }
                    else {
                        printf("Out of state.\n");
                        kickOut(client);
                    }

                    memset(client_message, 0, MAX_LENGTH_MESSAGE);
                    memset(server_message, 0, MAX_LENGTH_MESSAGE);
                    if (message) {
                        free_message(message);
                    }
                    break;

                case ERROR_PREFIX:
                    printf("Error occurred");
                    if (message->number_of_arguments == 1) {
                        printf("Error: %s", message->arguments[0]);
                    }
                    leave_the_game(client);

                    break;

                case PING_PREFIX:
                    printf("%s ponged\n", client->name);
                    pthread_mutex_lock(&mutex);
                    client->ponged = 0;     // yes
                    pthread_mutex_unlock(&mutex);


                    memset(client_message, 0, MAX_LENGTH_MESSAGE);
                    memset(server_message, 0, MAX_LENGTH_MESSAGE);
                    if (message) {
                        free_message(message);
                    }
                    break;

            }


        }
        else {
            printf("Error: Incorrect message \n");
            kickOut(client);
            memset(server_message, 0, MAX_LENGTH_MESSAGE);
        }


        // makes the client message all 0
        memset(client_message, 0, MAX_LENGTH_MESSAGE);
    }

    if(read_size == 0) {
        printf("Read size 0");
        //leave_the_game(client);
        //close(client->sock_id);
    }
    if(read_size == -1) {
        printf("Read size -1");
        //leave_the_game(client);
        //close(client->sock_id);
    }

    fflush(stdout);

    //free(new_client);

    return 0;
}

/**
 * Check if input is a number.
 */
int isANumber(char *input) {
    int length;

    length = strlen (input);
    for (int i = 0; i < length; i++)
        if (!isdigit(input[i])) {
            return -1;
        }
    return 0;
}


/**
 * Vytiskne info o tom jak spustit sever
 */
void arg_info(){
	printf("How to start a server\n");
	printf("-address || -a [IPv4] : set listening address (valid IPv4), default INADDR_ANY\n");
	printf("-port || -p [number] : set listening port (1024-49151), default 40000\n");
    printf("-games || -g [number] : set number of games (1-20), default 5\n");
    printf("-players || -pl [number] : set max number of players is the game (2-5), default 3\n");
	printf("Example: ./server.exe -a 127.0.0.1 -p 10000 -g 5 -pl 3\n\n");
}

/**
 * Funkce, která vytiskne do konzole, na obrazovku stav her/klientů
 */
void print_server() {
    printf("Games: \n");
    print_games(list_of_games, number_of_games);
    printf("Clients: \n");
    print_client_list(list_of_clients);
}


/**
 * Odstraneni klienta ze hry
 *
 * @param client Klient, který odeše lze hry
 */
void leave_the_game(Client *client) {
    printf("Client left the game entirely.\n");
    close(client->sock_id);
    remove_client(list_of_clients, client);
}

/**
 *  Odstraneni klienta ze hry
 *  @param client Klient, který odeše lze hry
 */
void kickOut(Client *client) {
    printf("Client was kicked out.\n");
    close(client->sock_id);
    remove_client(list_of_clients, client);
}