//
// Created by terez on 1/15/2021.
//

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "message_in.h"

/**
 * Funkce, která vytvoří strukturu (Message) zprávy.
 *
 * @return Struktura zprávy (Message)
 */
Message_in *create_message_in() {
    Message_in *message = (Message_in *) malloc(sizeof(Message_in));

    if (message) {
        message->number_of_arguments = 0;
        message->arguments = malloc(MAX_ARGUMENTS * sizeof(char *));

        for (int i = 0; i < MAX_ARGUMENTS; i++) {
            message->arguments[i] = malloc((MAX_LENGTH_OF_ARGUMENT + 1) * sizeof(char));
        }

    } else {
        printf("Error creating message");
    }
    return message;
}

/**
 * Zvaliduje příchozí zprávu a parsuje ji.
 *
 * Funkce ověří:
 *  - NULL check
 *  - déku min/max
 *  - prefix
 *  - obsahující ukončovací znak
 *  - min počet a max počet slov
 *
 * @param receive_message Příchozí zpráva, kteou vyslal klient
 * @return Strukturu zprávy Message_in nebo NULL
 */
Message_in *parse_in_message(char *received) {
    Message_in *message;
    printf("parse_in");

    if (received != NULL && strlen(received) >= MIN_LENGTH_MESSAGE && strlen(received) <= MAX_LENGTH_MESSAGE) {
        // if prefix exists
        if (is_prefix_correct(received[0]) == EXIT_SUCCESS) {
            // create new message
            message = create_message_in();

            if (message) {
                message->number_of_arguments = split_message(received, message->arguments);
                if (message->number_of_arguments != -1) {
                    message->prefix = received[0];
                    return message;
                } else {
                    free_message(message);
                    return NULL;
                }
            }
            else {
                free_message(message);
                return NULL;
            }
        } else {
            printf("Error parsing message. Message doesn't have correct prefix.\n");
            return NULL;
        }

    } else {
        printf("Error parsing message. Message is either NULL or message length is incorrect.\n");
        return NULL;
    }
}

/***
 * Funkce, která oddělí zprávu podle oddělovačů, který jsou definovány konstantou SEP a každá zpráva by měla obsahovat ukončovací znak END_CHAR.
 * Pokud je zpráva validní, tak funkce vrací počet slov a slova jsou uloženy do pole, který je předán parametrem.
 * Konstanty jsou definovány v modulu const.h
 *
 *
 * @param message příchozí zpráva
 * @param array pole slov, do kterého jso uslova uložena
 * @param token oddělovač
 *
 * @return Počet slov nebo kosntantu BAD_FOR_MESSAGE informující špatný formát zprávy
 */
int split_message(char *message, char **array) {
    int n = 0;      // index pro slovo
    int j = 0;      // index pro znak ve slově
    int i = 1;      // první znak je prefix

    if (message[i] != ' ' && message[i] != END_CHAR) {
        printf("ERROR: [Split message] in the %d. index not found token. \n\n", i);
        return -1;
    }

    while (message[i] != END_CHAR) {
        if (i > MAX_ARGUMENTS * MAX_LENGTH_OF_ARGUMENT) {
            printf("Error: [Split message] message is long \n");
            return -1;
        }

        if (n > MAX_ARGUMENTS) {
            printf("Error: [Split message] count word in message is more than %d \n\n", MAX_ARGUMENTS);
            return -1;
        }

        if (message[i] == '\0') {
            printf("Error: [Split message] didn't find end char ('%c'). \n\n", END_CHAR);
            return -1;
        }

        if (message[i] != ' ') {
            array[n][j++] = message[i];
            if (j > MAX_LENGTH_OF_ARGUMENT) {
                printf("Error: [Split message] word is longer than %d \n\n", MAX_LENGTH_OF_ARGUMENT);
                return -1;
            }
        } else {
            //konec slova
            if (j != 0) {
                array[n][j++] = '\0'; //insert NULL
                n++;
                j = 0;
            }
        }
        i++;
    }


    if (j != 0) {
        array[n][j++] = '\0'; //insert NULL
    }

    if (n == 0 && j == 0) {
        return 0; //pocet-slov
    } else {
        return (n + 1); //pocet slov - 0 je index
    }
}

/**
 * Funkce, která zvaliduje, jestli prvn íznak zprávy je validní prefix, pokud ne, tak příchozí zpráva je ve špatné formátu
 *
 * @param prefix - první znak zprávy
 * @return  EXIT_SUCCESS (obsahující prefix) nebo EXIT_FAILURE
 */
int is_prefix_correct(char prefix) {
    switch (prefix) {
        case 'I':
            return EXIT_SUCCESS; //LOGIN - screen
        case 'L':
            return EXIT_SUCCESS; //ROOM - screen
        case 'G':
            return EXIT_SUCCESS; //GAME - screen
        case 'E':
            return EXIT_SUCCESS; //ERROR
        default:
            return EXIT_FAILURE;
    }
}

/***
 * Funkce, která vytiskne informaci o zprávě (počet slov a všechny slova) na obazovku
 *
 * @param message Struktura zprávy Message_s
 */
void print_message(Message_in *message) {
    printf("[MESSAGE](%d)(prefix %c):", message->number_of_arguments, message->prefix);

    for (int i = 0; i < message->number_of_arguments; i++) {
            printf("'%s', ", message->arguments[i]);
    }
}

/**
 * Funkce, která uvolní paměť strukutry  zprávy Message
 *
 * @param message  Struktura zprávy Message
 */
void free_message(Message_in *message) {
    if (message) {
        if (message->arguments) {
            for (int i = 0; i < message->number_of_arguments; i++) {
                char *currentIntPtr = message->arguments[i];
                free(currentIntPtr);
            }
        }
        message = NULL;
        free(message);
    }
}