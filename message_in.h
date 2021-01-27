/*
 *
 *  Semestralni prace z predmetu UPS
 *  Autor: Tereza Tothova
 *  Datum: 10. 12. 2020
 *
 */

#ifndef SP1_MESSAGE_IN_H
#define SP1_MESSAGE_IN_H

//Definování délky zprávy
#define MAX_LENGTH_MESSAGE 100
#define MIN_LENGTH_MESSAGE 2
#define MAX_LENGTH_OF_ARGUMENT 20
#define MAX_ARGUMENTS 4
#define END_CHAR ';'        // ukoncovaci znak


// Prefixy

#define LOGIN_PREFIX 'I'    // message from login screen
#define GAME_PREFIX 'G'     // message from game screen
#define LOBBY_PREFIX 'L'    // message from lobby screen
#define PING_PREFIX 'P'     // ping message
#define ERROR_PREFIX 'E'    // error message

typedef struct Message_in {
    char prefix;
    char **arguments;
    int number_of_arguments;

} Message_in;

Message_in *create_message_in();
Message_in *parse_in_message(char received[MAX_LENGTH_MESSAGE]);
int split_message(char *message, char **array);
int is_prefix_correct(char prefix);
void free_message(Message_in *message);
void print_message(Message_in *message);


#endif //SP1_MESSAGE_IN_H
