#include <curses.h>
#include <form.h>
#include <menu.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <panel.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "game.h"
#include "ui.h"

#define CLEAR "clear"
#define MAX_CONNECTION_TRIES 3
#define RETRY_DELAY 1000

#define MENU_SINGLEPLAYER " Singleplayer "
#define MENU_MULTIPLAYER " Multiplayer "
#define LEADER_BOARD " Leaderboard "
#define RECONNECT " Connect "
#define QUIT " Quit "
#define CONTROLS " Controls "

enum ClientState
{
	EXIT = -1,
	MAIN_MENU = 0,
	SINGLEPLAYER_GAME = 1,
	MULTIPLAYER_GAME = 2,
	LEADERBOARD_MENU = 3,
	CONTROLS_MENU = 4
};

struct App* init_app()
{
	int n_states = 2;
	int n_high_scores = 10;
	struct App* app = (struct App*)malloc(sizeof(struct App));

	app->running = true;
	app->in_queue = false;
	app->player = 0;
	app->game_mode = SINGLEPLAYER;
	app->connection = get_connection();

	app->game_states = (struct GameState**)malloc(n_states * sizeof(struct GameState));
	for(int i = 0; i < n_states; i++)
	{
		app->game_states[i] = initialize_game_state();
	}

	struct HighScore** high_scores =
		(struct HighScore**)malloc(n_high_scores * sizeof(struct HighScore*));
	for(int i = 0; i < n_high_scores; i++)
	{
		high_scores[i] = (struct HighScore*)malloc(sizeof(struct HighScore));
		strcpy(high_scores[i]->name, "no name");
		high_scores[i]->score = 0;
	}
	app->scores = high_scores;

	return app;
}

void free_app(struct App* app)
{
	app->running = false;

	for(int i = 0; i < 10; i++)
	{
		free(app->scores[i]);
	}

	for(int i = 0; i < 2; i++)
	{
		free(app->game_states[i]->board);
		free(app->game_states[i]);
	}

	free(app->connection);
}

enum ClientState handle_main_menu(WINDOW* windows[N_WINDOWS], MENU* menu,
								  struct Connection* connection)
{
	ITEM* cur_item;

	switch(getch())
	{
	case KEY_DOWN:
		menu_driver(menu, REQ_DOWN_ITEM);
		break;
	case KEY_UP:
		menu_driver(menu, REQ_UP_ITEM);
		break;
	case 10:
		cur_item = current_item(menu);
		if(strcmp((char*)item_name(cur_item), MENU_SINGLEPLAYER) == 0)
		{
			if(!connection->open || connection->awaiting_connect_response)
			{
				mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "No connection to server.");
				break;
			}
			mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "Starting game...");
			return SINGLEPLAYER_GAME;
		}
		else if(strcmp((char*)item_name(cur_item), MENU_MULTIPLAYER) == 0)
		{
			if(!connection->open || connection->awaiting_connect_response)
			{
				mvwprintw(windows[MENU_WINDOW], 8, 5, "%s", "No connection to server.");
				break;
			}
			mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "Entering queue...");
			return MULTIPLAYER_GAME;
		}
		else if(strcmp((char*)item_name(cur_item), LEADER_BOARD) == 0)
		{
			if(!connection->open || connection->awaiting_connect_response)
			{
				mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "No connection to server.");
				break;
			}
			mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "Loading leaderboard...");
			return LEADERBOARD_MENU;
		}
		else if(strcmp((char*)item_name(cur_item), RECONNECT) == 0)
		{
			if(connection->open)
			{
				mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "Already connected...");
				break;
			}

			mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "Reconnecting...");
			send_connect(connection);
			break;
		}
		else if(strcmp((char*)item_name(cur_item), CONTROLS) == 0)
		{
			return CONTROLS_MENU;
		}
		else if(strcmp((char*)item_name(cur_item), QUIT) == 0)
		{
			mvwprintw(windows[MENU_WINDOW], 9, 5, "%s", "Quitting game...");
			return EXIT;
		}

		break;
	}

	return MAIN_MENU;
}

int main()
{
	struct App* app = init_app();

	pthread_t thread_id;
	int thread_created = pthread_create(&thread_id, NULL, listen_on_connection, app);
	if(thread_created != 0)
	{
		printf("[CLIENT] Failed to start thread.\n");
		exit(EXIT_FAILURE);
	}

	send_connect(app->connection);

	setup_ui();

	WINDOW* windows[N_WINDOWS];
	PANEL* panels[N_WINDOWS];

	char* menu_items[N_MENU_ITEMS] = {
		MENU_SINGLEPLAYER, MENU_MULTIPLAYER, LEADER_BOARD, CONTROLS, RECONNECT, QUIT};
	MENU* menu;

	FIELD* fields[4];
	FORM* form;

	fields[0] = new_field(1, 3, 0, 1, 0, 0);
	fields[1] = new_field(1, 20, 0, 5, 0, 0);
	fields[2] = new_field(1, 10, 0, 25, 0, 0);
	fields[3] = NULL;

	menu = create_menu(menu_items);
	form = create_highscore_form(fields);

	create_windows(windows, panels, menu, form);

	show_panel(panels[HEADER_WINDOW]);

	enum ClientState state = MAIN_MENU;

	while(state != EXIT)
	{
		state = MAIN_MENU;

		mvwprintw(windows[MENU_WINDOW], 9, 5, "%24s", "");
		wrefresh(windows[MENU_WINDOW]);
		show_panel(panels[MENU_WINDOW]);
		hide_panel(panels[GAME_WINDOW]);
		hide_panel(panels[OPPONENT_WINDOW]);
		hide_panel(panels[SCORE_WINDOW]);
		hide_panel(panels[OPPONENT_SCORE_WINDOW]);
		hide_panel(panels[LEADERBOARD_WINDOW]);
		hide_panel(panels[HIGHSCORE_WINDOW]);
		hide_panel(panels[QUEUE_WINDOW]);
		hide_panel(panels[PIECE_QUEUE_WINDOW]);
		hide_panel(panels[HOLD_PIECE_WINDOW]);
		hide_panel(panels[CONTROLS_WINDOW]);
		update_panels();

		while(state == MAIN_MENU)
		{
			state = handle_main_menu(windows, menu, app->connection);
			refresh_ui(app, windows);
		}

		if(state == SINGLEPLAYER_GAME)
		{
			app->game_mode = SINGLEPLAYER;
			play_singleplayer_game(app, windows, form, fields, panels);
		}

		if(state == MULTIPLAYER_GAME)
		{
			app->game_mode = MULTIPLAYER;
			send_queue(app, true);
			hide_panel(panels[MENU_WINDOW]);
			show_panel(panels[QUEUE_WINDOW]);
			update_panels();

			int action;

			while(app->in_queue)
			{
				action = getch();
				if(action == 'q')
				{
					send_queue(app, false);
					state = MAIN_MENU;
				}
				wrefresh(windows[QUEUE_WINDOW]);
			}

			if(state == MULTIPLAYER_GAME)
			{
				play_multiplayer_game(app, windows, panels);
			}
		}

		if(state == LEADERBOARD_MENU)
		{
			request_leaderboard(app->connection);
			sleep(1);
			print_leaderboard(windows[LEADERBOARD_WINDOW], app->scores);
			hide_panel(panels[MENU_WINDOW]);
			show_panel(panels[LEADERBOARD_WINDOW]);
			wrefresh(windows[LEADERBOARD_WINDOW]);
			update_panels();
			while(getch() != 'q')
			{
				wrefresh(windows[LEADERBOARD_WINDOW]);
			}
		}

		if(state == CONTROLS_MENU)
		{
			hide_panel(panels[MENU_WINDOW]);
			show_panel(panels[CONTROLS_WINDOW]);
			update_panels();
			while(getch() != 'q')
			{
				wrefresh(windows[CONTROLS_WINDOW]);
			}
		}
	}

	app->running = false;

	unpost_form(form);
	for(int i = 0; i < N_FIELDS; i++)
	{
		free_field(fields[i]);
	}
	free_form(form);

	unpost_menu(menu);
	for(int i = 0; i < N_MENU_ITEMS; i++)
	{
		free_item(menu->items[i]);
	}
	free_menu(menu);

	close_connection(app->connection, thread_id);
	free_app(app);
	endwin();
	system(CLEAR);
	exit(EXIT_SUCCESS);
}
