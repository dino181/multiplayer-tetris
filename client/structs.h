#ifndef STRUCTS_H
#define STRUCTS_H

struct HighScore{
	char name[20];
	int score;
};

enum GameMode
{
	SINGLEPLAYER = 0,
	MULTIPLAYER = 1,
};

struct App
{
	bool running;
	bool in_queue;
	int player;
	enum GameMode game_mode;
	struct Connection* connection;
	struct GameState** game_states;
	struct HighScore** scores;
};

struct GameState{
	int** board;
	bool game_over;
	int score;
	bool playing;
	bool winning;
	int piece_queue[3];
	int hold_piece;
};

#endif
