/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020
 *  Modul client.c
 *
 *
 *
 */

#include<unistd.h>

#include "client.h"
#include "game.h"



/**
 * Vytvoří linked list klientů.
 *
 * @return prázdný list klientů
 */
Client_list *create_client_list() {
    Client_list *list = (Client_list*) malloc(sizeof(Client_list));

    if (list) {
        list->first = NULL;
        list->last = NULL;
        list->size = 0;
    } else {
        printf("Error creating list of clients. ");
    }
    return list;
}

/***
 * Vytvoří strukturu klienta.
 *
 * @param name Jméno klienta
 * @param ip ip adresa klienta
 * @param sock_id id socketu klienta
 *
 * @return klient
 */
Client *create_client(char name[20], char ip[20], int sock_id) {
    Client *newClient;

    newClient = (Client *) malloc(sizeof(Client));

    if (newClient) {
        strcpy(newClient->name, name);
        strcpy(newClient->client_ip, ip);

        newClient->sock_id = sock_id;

        newClient->game_id = -1;

        newClient->state = CLIENT_STATE_LOGIN;

        newClient->number_of_cards_in_hand = 0;
        Card *cards = (Card*)malloc(MAX_CARDS_IN_HAND * sizeof(Card));
        newClient->cards = cards;

        newClient->wants_another_card = 0;  // yes

        newClient->next = NULL;
        newClient->previous = NULL;

    } else {
        printf("Error: Error creating client");
    }
    return newClient;
}

/***
 * Přidá klienta do listu.
 *
 * @param list List klientů
 * @param client klient
 *
 * @return EXIT_SUCCESS nebo EXIT_FAILURE
 */
int add_client(Client_list *list, Client *client) {

    if (list->size == 0) {
        list->first = client;
        list->last = client;
    } else {
        list->last->next = client;
        client->previous = list->last;
        list->last = client;
    }

    list->size++;

    return EXIT_SUCCESS;
}

/**
 * Vypíše informace o klientovi
 *
 * @param client klient
 */
void print_client(Client *client) {
    if (client) {
        printf("	%s", client->name);
        printf("(ip: %s) ", client->client_ip);
        printf("(game_id: %d) ", client->game_id);
        printf("(state: %d)\n", client->state);
    } else {
        printf("Error printing client)\n");
    }
}

/**
 * Vypíše informace o všech klientech v listu
 *
 * @param list List, který chceme vypsat
 */
void print_client_list(Client_list *list) {
    if (list) {
        Client *tmp = list->first;
        while (tmp) {
            print_client(tmp);
            tmp = tmp->next;
        }
    } else {
        printf("Error: printing list\n");
    }
    printf("\n");
}

/**
 * Vrátí referenci na klienta v listu podle jména.
 *
 * @param list List klientů
 * @param name Jméno klienta
 *
 * @return Vrácíme nalezeného klienta a nebo NULL
 */
Client *get_client_by_name(struct Client_list *list, char name[20]) {
    Client *tmp = list->first;

    if (list && name) {
        if (list->size != 0) {
            while (tmp != NULL) {
                if (strcmp(tmp->name, name) == 0) {
                    return tmp;
                }
                tmp = tmp->next;
            }
            return NULL;
        } else {
            return NULL;
        }

    } else {
        return NULL;
    }
}

/**
 *  Nastaví jméno klienta
 *
 * @param list_of_clients List všech klientů připojených an server
 * @param client klient
 * @param message zpráva která přišla, obsahující jméno s prefixe LOGIN
 *
 * @return EXIT_SUCCESS nebo EXIT_FAILURE
 */
int set_name(Client_list *list_of_clients, Client *client, Message_in *message) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    if (list_of_clients && client && message) {
        // incorrect form of the message
        if (message->number_of_arguments != 1) {
            printf("[LOGIN]: Incorrect form of message\n");
            return EXIT_FAILURE;
        }

        char * name;
        name = message->arguments[0];
        int length = strlen(name);

        // name too short
        if(length < MIN_NAME_LENGTH) {
            name[length] = '1';
            name[length+1] = '\0';
        }

        // name already exists, add 1
        if(get_client_by_name(list_of_clients, name) != NULL) {
            if(length == MAX_NAME_LENGTH) {
                name[length-1] = '1';
                name[length] = '\0';
            }
            else {
                name[length] = '1';
                name[length+1] = '\0';
            }
        }

        // name is OK
        printf("[LOGIN]: %s\n", name);

        strcpy(client->name, name);
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %s%c", LOGIN_PREFIX, "logged_in", client->name, END_CHAR);
        send_message_to_client(client->sock_id, server_message);
        memset(server_message, 0, MAX_LENGTH_MESSAGE);

        return EXIT_SUCCESS;
    } else {
        printf("Error setting name of the client)\n");
        return EXIT_FAILURE;
    }
}

/**
 * Odstraní klienta z linked listu.
 *
 * @param list List klientů
 * @param client klient
 *
 * @return  EXIT_FAILURE nebo EXIT_SUCCESS
 */
int remove_client(Client_list *list, Client *client) {

    if(client && list) {

        printf("Deleting client");

        // first in the list
        if(client == list->first) {
            if(client->next != NULL) {
                list->first = client->next;
            }
            else {
                list->first = NULL;
            }
        }

        // last in the list
        if(client == list->last) {
            if(client->previous != NULL) {
                list->last = client->previous;
            }
            else {
                list->last = NULL;
            }
        }

        if(client->next != NULL && client->previous != NULL)  {
            client->previous->next = client->next;
            client->next->previous = client->previous;
        }

        free(client);
        list->size--;

        return EXIT_SUCCESS;
    }

    printf("Error deleting client\n");
    return EXIT_FAILURE;
}

/**
 * Umožňuje poslat klientovy s daným sosket_id zprávu.
 *
 * @param sock_id  socket id klienta
 * @param message  zpráva
 */
void send_message_to_client(int sock_id, char *message) {
    char send_message[MAX_LENGTH_MESSAGE];

    snprintf(send_message, MAX_LENGTH_MESSAGE, "%s\n", message);
    printf("[SENDING]: %s", send_message);
    write(sock_id, send_message, strlen(send_message));

    memset(send_message, 0, MAX_LENGTH_MESSAGE);
}