#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "game.h"

static char* trim_whitespaces(char* str)
{
	char* end;
	while(isspace(*str))
		str++;

	if(*str == 0)
		return str;

	end = str + strnlen(str, 128) - 1;
	while(end > str && isspace(*end))
		end--;

	*(end + 1) = '\0';

	return str;
}

int get_rank(struct HighScore** high_scores, int score)
{
	for(int i = 0; i < 10; i++)
	{
		if(high_scores[i]->score < score)
		{
			return i;
		}
	}
	return -1;
}

void play_game(struct App* app, WINDOW* windows[N_WINDOWS])
{
	app->game_states[0]->playing = true;
	app->game_states[0]->game_over = false;
	app->game_states[1]->playing = true;
	app->game_states[1]->game_over = false;

	while(app->connection->open)
	{
		if(app->game_mode == SINGLEPLAYER && app->game_states[app->player]->game_over)
		{
			app->game_states[app->player]->playing = false;
			break;
		}

		if(app->game_mode == MULTIPLAYER)
		{
			if(!(app->game_states[app->player]->playing) &&
			   app->game_states[1 - app->player]->game_over)
			{
				app->game_states[1 - app->player]->playing = false;
				app->game_states[app->player]->playing = false;
				if(app->game_states[app->player]->winning)
				{
					print_win(windows[GAME_WINDOW], app->game_states[app->player]->board, false);
					print_loss(
						windows[OPPONENT_WINDOW], app->game_states[1 - app->player]->board, false);
					sleep(3);
				}
				else
				{
					print_loss(windows[GAME_WINDOW], app->game_states[app->player]->board, false);
					print_win(
						windows[OPPONENT_WINDOW], app->game_states[1 - app->player]->board, false);
					sleep(3);
				}
				break;
			};

			if(!(app->game_states[app->player]->playing))
			{
				refresh_ui(app, windows);
				usleep(1000 * 100);
				continue;
			}

			if(app->game_states[app->player]->game_over)
			{
				app->game_states[app->player]->playing = false;
			};
		}

		refresh_ui(app, windows);

		switch(getch())
		{
		case KEY_LEFT:
			send_action(app->connection, 'l');
			break;
		case KEY_RIGHT:
			send_action(app->connection, 'r');
			break;
		case KEY_DOWN:
			send_action(app->connection, 'd');
			break;
		case ' ':
			send_action(app->connection, 's');
			break;
		case 'e':
			send_action(app->connection, 'e');
			break;

		case 'q':
			send_action(app->connection, 'q');
			break;
		case 'w':
			send_action(app->connection, 'h');
			break;
		default:
			break;
		}
	}
}

int** create_board()
{
	int** board = (int**)malloc(BOARD_HEIGHT * sizeof(int*));
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		board[i] = (int*)malloc(BOARD_WIDTH * sizeof(int));
	};
	return board;
}

struct GameState* initialize_game_state()
{
	struct GameState* game_state = (struct GameState*)malloc(sizeof(struct GameState));

	game_state->board = create_board();
	game_state->game_over = false;
	game_state->score = 0;
	game_state->playing = false;
	game_state->winning = false;
	game_state->piece_queue[0] = 0;
	game_state->piece_queue[1] = 0;
	game_state->piece_queue[2] = 0;
	game_state->hold_piece = 1;

	return game_state;
}

void play_multiplayer_game(struct App* app, WINDOW* windows[N_WINDOWS], PANEL* panels[N_WINDOWS])
{
	show_panel(panels[GAME_WINDOW]);
	show_panel(panels[OPPONENT_WINDOW]);
	show_panel(panels[SCORE_WINDOW]);
	show_panel(panels[OPPONENT_SCORE_WINDOW]);
	show_panel(panels[PIECE_QUEUE_WINDOW]);
	show_panel(panels[HOLD_PIECE_WINDOW]);
	hide_panel(panels[QUEUE_WINDOW]);
	refresh_ui(app, windows);
	update_panels();
	play_game(app, windows);
}

void play_singleplayer_game(struct App* app, WINDOW* windows[N_WINDOWS], FORM* form,
							FIELD* fields[4], PANEL* panels[N_WINDOWS])
{
	hide_panel(panels[MENU_WINDOW]);
	show_panel(panels[GAME_WINDOW]);
	show_panel(panels[SCORE_WINDOW]);
	show_panel(panels[PIECE_QUEUE_WINDOW]);
	show_panel(panels[HOLD_PIECE_WINDOW]);
	refresh_ui(app, windows);
	update_panels();

	send_start(app->connection);
	play_game(app, windows);

	request_leaderboard(app->connection);
	print_game_over(windows[GAME_WINDOW], app->game_states[0]->board);
	print_leaderboard(windows[LEADERBOARD_WINDOW], app->scores);

	hide_panel(panels[GAME_WINDOW]);
	hide_panel(panels[SCORE_WINDOW]);
	hide_panel(panels[PIECE_QUEUE_WINDOW]);
	hide_panel(panels[HOLD_PIECE_WINDOW]);
	show_panel(panels[LEADERBOARD_WINDOW]);

	update_panels();

	int rank = get_rank(app->scores, app->game_states[0]->score);
	if(rank != -1)
	{
		curs_set(1);
		char rank_str[4] = {0};
		char score_str[5] = {0};
		sprintf(rank_str, "%2d.", rank + 1);
		sprintf(score_str, "%4d", app->game_states[0]->score);
		set_field_buffer(fields[0], 0, rank_str);
		set_field_buffer(fields[2], 0, score_str);
		show_panel(panels[HIGHSCORE_WINDOW]);

		int ch;
		while(1)
		{
			ch = getch();
			form_driver(form, REQ_VALIDATION);

			int length = strlen(trim_whitespaces(field_buffer(fields[1], 0)));

			if(ch == 10 && length > 0)
			{
				break;
			}

			driver_form(form, ch, length);
			wrefresh(windows[HIGHSCORE_WINDOW]);
		}

		send_highscore_name(app->connection, trim_whitespaces(field_buffer(fields[1], 0)));
		set_field_buffer(fields[1], 0, "");
		curs_set(0);
	}
	else
	{
		while(getch() != 'q')
		{
			wrefresh(windows[LEADERBOARD_WINDOW]);
		}
	}
}
