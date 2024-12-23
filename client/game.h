#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <panel.h>
#include <form.h>

#include "constants.h"
#include "client.h"
#include "ui.h"
#include "structs.h"

int** create_board();

struct GameState* initialize_game_state();

void play_singleplayer_game(struct App* app, WINDOW* windows[N_WINDOWS], FORM* form, FIELD* fields[4], PANEL* panels[N_WINDOWS]);

void play_multiplayer_game(struct App* app, WINDOW* windows[N_WINDOWS], PANEL* panels[N_WINDOWS]);


#endif
