/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020
 *  Modul message_in.c
 *
 *
 *
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "message_in.h"

/**
 * Vytvoří strukturu zprávy - Message_in
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
 * Parsuje a validuje zprávu.
 *
 * @param receive_message přijatá zpráva
 *
 * @return Message_in zpráva nebo NULL
 */
Message_in *parse_in_message(char *received) {
    Message_in *message;
    printf("parse_in\n");

    if (received != NULL && strlen(received) >= MIN_LENGTH_MESSAGE && strlen(received) <= MAX_LENGTH_MESSAGE) {
        // if prefix exists
        if (is_prefix_correct(received[0]) == EXIT_SUCCESS) {
            printf("prefix correct\n");
            // create new message
            message = create_message_in();

            // if message exists
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
 * Odděluje argumenty od sebe podle mezer a koncových znaků
 *
 *
 * @param message příchozí zpráva
 * @param array pole argumentů
 *
 * @return Počet slov nebo -1
 */
int split_message(char *message, char **array) {
    int n = 0;      // index pro slovo
    int j = 0;      // index pro znak ve slově
    int i = 1;      // první znak je prefix

    if (message[i] != ' ' && message[i] != END_CHAR) {
        printf("ERROR: [Split message] in the %d. char should be space. Incorrect message. \n", i);
        return -1;
    }

    while (message[i] != END_CHAR) {
        if (i > MAX_ARGUMENTS * MAX_LENGTH_OF_ARGUMENT) {
            printf("Error: [Split message] message is too long \n");
            return -1;
        }

        if (n > MAX_ARGUMENTS - 1) {
            printf("Error: [Split message] too many arguments in message \n");
            return -1;
        }

        if (message[i] == '\0') {
            printf("Error: [Split message] didn't find end char ('%c'). \n\n", END_CHAR);
            return -1;
        }

        if (message[i] != ' ') {
            array[n][j] = message[i];
            j++;
            if (j > MAX_LENGTH_OF_ARGUMENT) {
                printf("Error: [Split message] argument too long \n");
                return -1;
            }
        } else {
            // konec slova
            if (j != 0) {
                array[n][j] = '\0';     //insert NULL
                n++;
                j = 0;
            }
        }
        i++;
    }


    if (j != 0) {
        array[n][j++] = '\0';   //insert NULL
    }

    if (n == 0 && j == 0) {
        return 0;
    } else {
        return (n + 1);
    }
}

/**
 * Zvaliduje, že daný prefix existuje.
 *
 * @param prefix - první znak zprávy
 *
 * @return  EXIT_SUCCESS nebo EXIT_FAILURE
 */
int is_prefix_correct(char prefix) {
    switch (prefix) {
        case 'I':
            return EXIT_SUCCESS;
        case 'L':
            return EXIT_SUCCESS;
        case 'G':
            return EXIT_SUCCESS;
        case 'E':
            return EXIT_SUCCESS;
        case 'P':
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
    }
}

/***
 * Vvytiskne informaci o zprávě
 *
 * @param message Struktura zprávy Message_in
 */
void print_message(Message_in *message) {
    printf("[MESSAGE](%d)(prefix %c):", message->number_of_arguments, message->prefix);

    for (int i = 0; i < message->number_of_arguments; i++) {
            printf("'%s', ", message->arguments[i]);
    }
    printf("\n");
}

/**
 * Uvolní message_in
 *
 * @param message  Message_in struct
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
