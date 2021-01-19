//
// Created by terez on 1/13/2021.
//

#include "game.h"

/**
 * Inicilaizuje list místností a naplní ho určeným počtem místností z parametru countGames.

 * @param countGames počet her
 *
 * @return lsit napněn místsnosti (struktura game_room)
 */
Game *create_games(int countGames, int countPlayers) {
    Game *games = (Game*)malloc(countGames * sizeof(Game));

    for(int i = 0; i < countGames; i++) {
        games[i].id_game = i;
        games[i].state = GAME_STATE_NONE;
        games[i].number_of_players = 0;

        // malloc clients
        Client **players = (Client**)malloc(countPlayers * sizeof(Client*));
        games[i].list_of_players = players;

        // malloc cards
        Card *cards = (Card*)malloc(NUMBER_OF_CARDS * sizeof(Card));
        games[i].cards = cards;

        games[i].index_of_the_card = 0;
    }

    return games;
}

/**
 * Přidá klienta do místnosti.
 *
 * @param id_room id hry/místnoti
 * @param list_of_games lsit všech místnotí
 * @param client kleint, který se rád přidal do hry
 * @param list_of_clients lsit všech klientů
 *
 * @return EXIT_SUCCESS neb o EXIT_FAILURE
 */
int add_player(Game *game,  Client *client, int players_in_game) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    if (game && client) {
        if (game->state == 0 || game->state == 1) {
            if (game->number_of_players < players_in_game) {

                client->game_id = game->id_game;

                game->state = GAME_STATE_WAITING;

                game->list_of_players[game->number_of_players] = client;
                game->number_of_players++;

                // game is full now
                if (game->number_of_players == players_in_game) {
                    game->state = GAME_STATE_FULL;
                }

                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %d %d%c", LOBBY_PREFIX, "entered", game->id_game, players_in_game, END_CHAR);
                send_message_to_client(client->sock_id, server_message);

            } else {
                printf("The game %d is full. Client can't be added.\n", game->id_game);

                snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %d%c", LOBBY_PREFIX, "game_full", game->id_game, END_CHAR);
                send_message_to_client(client->sock_id, server_message);

                return EXIT_FAILURE;
            }

        } else {
            printf("Error adding client to the game\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
}

/**
* Vrátí místnost podle id.
*
* @param games List všech her/místností
* @param id_game id hry/místnosti
* @return Struktura hry/místnosti (Game_room_s)
*/
Game *get_game(Game *games, int count, int id_game) {

    for(int i = 0; i < count; i++) {
        if(games[i].id_game == id_game) {
            return &games[i];
        }
    }
    return NULL;
}


/**
 * Vypíše informaci o každé místnosti na obrazovku
 *
 * @param list_of_games  List místnotí
 */
void print_games(Game *games, int count) {

    for(int i = 0; i < count; i++) {
        print_game(&games[i]);
    }

    printf("\n");
}

/**
 * Vytiskne informaci o hře na obrazovku
 *
 * @param game Game_room_s hra o které chceme vypsat info
 */
static void print_game(Game *game) {
    if (game) {
        printf("GameID: %d, state: %d, number of current players: %d", game->id_game, game->state, game->number_of_players);

        printf("	Clients:\n");

        for(int i = 0; i < game->number_of_players; i++) {
            printf("Client name: %s\n", game->list_of_players[i]->name);
        }

    } else {
        printf("Error printing game\n");
    }

}

/**
 * Pošle informaci o stavu místnosti danému klientovi se sock_id parametrem.
 * Metoda posílá zprávu ve tvaru: PREFIX+ODDELOVAC+INFO+ODDELOVAC+ID HRY+ODDELOVAC+STAV HRY+ODDELOVAC+POCET HRACU VE HRE+UKONCOVACi ZNAK
 *
 * @param sock_id Sock id klienta
 * @param list_of_games  List všech her/místnosté
 * @param id_game  id hry/místnosti o které chci poslat informaci
 */
void send_game_info(int sock_id, Game **list_of_games, int game_id, int count) {
    char server_message[MAX_LENGTH_MESSAGE];
    memset(server_message, 0, MAX_LENGTH_MESSAGE);

    Game *game = get_game(*list_of_games, count, game_id);

    if (game) {
        //message - ROOMS#INFO#id_room#state#..
        snprintf(server_message, MAX_LENGTH_MESSAGE, "%c %s %d %d %d%c", LOBBY_PREFIX, "info", game_id,
                 game->state, game->number_of_players, END_CHAR);
        send_message_to_client(sock_id, server_message);
    }
}