//
// Created by terez on 1/12/2021.
//

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
 *  Vytvoří strukturu klienta.
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
 * @param list List, do kterého chci přidat strukturu klienta
 * @param client struktura klienta, kterého chci přidat do listu
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
 * Vytiskne informace o klientovi do konzole
 *
 * @param client Struktura klienta, kterou chceme vytisknout
 */
void print_client(Client *client) {
    if (client) {
        printf("	%s", client->name);
        printf("(ip: %s) ", client->client_ip);
        printf("(id: %d) ", client->game_id);
        printf("(state: %d)\n", client->state);
    } else {
        printf("Error printing client)\n");
    }
}

/**
 * Vytiskne na obrazovku informace o každém klientovi v listu.
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
 * Metoda vrátí referenci na klienta v listu podle jména.
 *
 * @param list List klientů, ve kterém hledáme daného klienta podle jména
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
 *  Metoda, která zvaliduje jméno klienta.
 *  Pokud už dané jméno existuje, tak metoda pošle zprávu klientovi, že jméno je použito, jinak pošle že akceptováno.
 *  Pokud nastaane jiná chyba, např, delší jméno nebo krátký, tak metoda vrátí EXIT_FAILURE.
 *
 * @param list_of_clients List všech klientů připojených an server
 * @param client daný klient který se chce připojit
 * @param message zpráva která přišla, obsahující jméno s prefixe LOGIN
 * @return 0  = added, 1 = jméno je použito  nebo EXIT_FAILURE
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
 * Umožňuje poslat klientovy s daným soket id zprávu.
 *
 * @param sock_id  soket id klienta
 * @param message  zpráva
 */
void send_message_to_client(int sock_id, char *message) {
    char send_message[MAX_LENGTH_MESSAGE];

    snprintf(send_message, MAX_LENGTH_MESSAGE, "%s\n", message);
    printf("[SENDING]: %s", send_message);
    write(sock_id, send_message, strlen(send_message));

    memset(send_message, 0, MAX_LENGTH_MESSAGE);
}