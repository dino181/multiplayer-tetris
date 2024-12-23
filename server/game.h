#ifndef GAME_H
#define GAME_H

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define NO_INPUT_CHAR '$'

#include <stdbool.h>

struct GameState {
	int** board;
	int game_over;
	int score;
	char current_action;
	int playing;
	int winning;
	int piece_queue[3];
	int hold_piece;
	int held_piece;
	bool use_held_piece;
	bool can_hold;
	int level;
};

struct SingleplayerGame {
	int server;
	struct Client* client;
};

struct MultiplayerGame {
	int server;
	struct Client* players[];
};

struct SingleplayerGame* initialize_game(int server, struct Client* client);

struct MultiplayerGame* initialize_multiplayer_game(int server, struct Client* player1, struct Client* player2);

int** create_board();

void* run_singleplayer_game(void *args);

void* run_multiplayer_game(void *args);

#endif
