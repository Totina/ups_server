/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020

 *
 */

#ifndef SP1_CLIENT_H
#define SP1_CLIENT_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "message_in.h"
#include "cards.h"

#define MIN_NAME_LENGTH 2
#define MAX_NAME_LENGTH 20
#define MAX_CARDS_IN_HAND 12

typedef enum client_state {
    CLIENT_STATE_LOGIN,
    CLIENT_STATE_LOBBY,
    CLIENT_STATE_WAITING,
    CLIENT_STATE_IN_GAME,
    CLIENT_STATE_DISCONNECTED
} client_state;

// Struktura Klient
typedef struct Client {
    char name[20];
    char client_ip[20];
    int sock_id;

    int game_id;

    enum client_state state;

    int number_of_cards_in_hand;
    Card *cards;

    int wants_another_card;     // 0 yes (default), 1 no

    struct Client *next;
    struct Client *previous;

} Client;

// Struktura list klientu
typedef struct Client_list {
    Client *first;
    Client *last;
    int size;
} Client_list;

// funkce
Client_list *create_client_list();
Client *create_client(char name[20], char ip[20], int sock_id);
int add_client(Client_list *list, Client *client);
void print_client(Client *client);
void print_client_list(Client_list *list);
Client *get_client_by_name(struct Client_list *list, char name[20]);
int set_name(Client_list *list_of_clients, Client *client, Message_in *message);
void send_message_to_client(int sock_id, char *message);
int remove_client(Client_list *list, Client *client);


#endif //SP1_CLIENT_H
