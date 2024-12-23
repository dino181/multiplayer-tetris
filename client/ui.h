#ifndef UI_H
#define UI_H

#include <menu.h>
#include <form.h>
#include <panel.h>

#include "client.h"
#include "constants.h"
#include "structs.h"

enum Windows{
	MENU_WINDOW = 0,
	GAME_WINDOW = 1,
	HEADER_WINDOW = 2,
	SCORE_WINDOW = 3,
	LEADERBOARD_WINDOW = 4,
	HIGHSCORE_WINDOW = 5,
	QUEUE_WINDOW = 6,
	OPPONENT_WINDOW = 7,
	OPPONENT_SCORE_WINDOW = 8,
	PIECE_QUEUE_WINDOW = 9,
	HOLD_PIECE_WINDOW = 10,
	CONTROLS_WINDOW = 11, 
};

enum ConnectionStatus{
	CONNECTING = 0, 
	DISCONNECTED =1 ,
	CONNECTED = 2,
};

void create_windows(WINDOW* windows[N_WINDOWS], PANEL* panels[], MENU* menu, FORM* form);

void print_board(WINDOW* window, int** board); 

void print_game_over(WINDOW* window, int** board);

void print_loss(WINDOW* window, int** board, bool with_animation);

void print_win(WINDOW* window, int** board, bool with_animation);

void print_leaderboard(WINDOW* window, struct HighScore** scores);

void print_highscore(WINDOW* window);

void setup_ui();

void refresh_ui(struct App* app, WINDOW* windows[N_WINDOWS]);

MENU* create_menu(char* choices[3]);

WINDOW* new_menu_window(MENU* menu);

WINDOW* new_game_window(int left_offset);

WINDOW* new_score_window(int left_offset);

WINDOW* new_header_window();

WINDOW* new_piece_queue_window();

WINDOW* new_hold_piece_window();

WINDOW* new_leaderboard_window();

WINDOW* new_queue_window();

WINDOW* new_highscore_window(FORM* form);

WINDOW* new_controls_window();

void cleanup_menu(WINDOW* menu_window, MENU* menu, ITEM** menu_items, int n_menu_items);

void driver_form(FORM* form, int ch, int name_length);

FORM* create_highscore_form(FIELD* fields[N_FIELDS]);

#endif
