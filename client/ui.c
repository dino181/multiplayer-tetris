#include "ui.h"
#include "constants.h"
#include <curses.h>
#include <form.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

#include "client.h"

#define BOARD_BORDER 1
#define LEFT_PADDING 5
#define PING_POSITION 16
#define HEADER_END 30

#define DEFAULT_TEXT_COLOR 9
#define PIECE_QUEUE_SIZE 3

void print_board(WINDOW* window, int** board)
{
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			wattron(window, COLOR_PAIR(board[i][j] + 1));
			mvwprintw(window, BOARD_BORDER + i * 2, BOARD_BORDER + j * 4, "%4d", board[i][j]);
			mvwprintw(window, BOARD_BORDER + i * 2 + 1, BOARD_BORDER + j * 4, "%4d", board[i][j]);
			wattroff(window, COLOR_PAIR(board[i][j] + 1));
		}
	}
}

void print_header(WINDOW* window)
{
	mvwhline(window, 1, LEFT_PADDING, ACS_HLINE, HEADER_END);
	mvwaddch(window, 1, LEFT_PADDING, ACS_LLCORNER);
	mvwhline(window, 0, LEFT_PADDING, ACS_VLINE, 1);
	mvwhline(window, 0, LEFT_PADDING + PING_POSITION, ACS_VLINE, 1);
	mvwaddch(window, 1, LEFT_PADDING + PING_POSITION, ACS_BTEE);
	mvwhline(window, 1, LEFT_PADDING + HEADER_END, ACS_LRCORNER, 1);
	mvwhline(window, 0, LEFT_PADDING + HEADER_END, ACS_VLINE, 1);
}

void print_connection_status(WINDOW* window, enum ConnectionStatus connection_status)
{
	int color_pair;
	char* status = "";
	switch(connection_status)
	{
	case CONNECTING:
		color_pair = DEFAULT_TEXT_COLOR;
		status = "Connecting...";
		break;
	case DISCONNECTED:
		color_pair = 10;
		status = "No connection";
		break;
	case CONNECTED:
		color_pair = 11;
		status = "Connected";
		break;
	}

	wattron(window, COLOR_PAIR(color_pair));
	mvwprintw(window, 0, LEFT_PADDING + 2, "%-13s", status);
	wattroff(window, COLOR_PAIR(color_pair));
}

void print_ping(WINDOW* window, int ping)
{
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	mvwprintw(window, 0, LEFT_PADDING + PING_POSITION + 2, "Ping %3d ms", ping);
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
}

void print_score(WINDOW* window, int score)
{
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	mvwprintw(window, 3, 2, "%-10d", score);
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
}

void print_leaderboard(WINDOW* window, struct HighScore** scores)
{
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	for(int i = 0; i < 10; i++)
	{
		mvwprintw(window, 5 + i, 2, "%2d. %-20s %d", i + 1, scores[i]->name, scores[i]->score);
	}
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
}

void print_piece(WINDOW* window, int x, int y, int piece)
{
	wattron(window, COLOR_PAIR(1));
	mvwprintw(window, y, x - 2, "%16d", 0);
	mvwprintw(window, y + 1, x - 2, "%16d", 0);
	mvwprintw(window, y + 2, x - 2, "%16d", 0);
	mvwprintw(window, y + 3, x - 2, "%16d", 0);
	wattroff(window, COLOR_PAIR(1));

	wattron(window, COLOR_PAIR(piece + 1));
	switch(piece)
	{
	case 1:
		mvwprintw(window, y + 1, x - 2, "%16d", 0);
		mvwprintw(window, y + 2, x - 2, "%16d", 0);
		break;
	case 2:
		mvwprintw(window, y, x, "%12d", 0);
		mvwprintw(window, y + 1, x, "%12d", 0);
		mvwprintw(window, y + 2, x, "%4d", 0);
		mvwprintw(window, y + 3, x, "%4d", 0);
		break;
	case 3:
		mvwprintw(window, y, x, "%12d", 0);
		mvwprintw(window, y + 1, x, "%12d", 0);
		mvwprintw(window, y + 2, x + 8, "%4d", 0);
		mvwprintw(window, y + 3, x + 8, "%4d", 0);
		break;
	case 4:
		mvwprintw(window, y, x + 4, "%8d", 0);
		mvwprintw(window, y + 1, x + 4, "%8d", 0);
		mvwprintw(window, y + 2, x, "%8d", 0);
		mvwprintw(window, y + 3, x, "%8d", 0);
		break;
	case 5:
		mvwprintw(window, y, x, "%8d", 0);
		mvwprintw(window, y + 1, x, "%8d", 0);
		mvwprintw(window, y + 2, x + 4, "%8d", 0);
		mvwprintw(window, y + 3, x + 4, "%8d", 0);
		break;
	case 6:
		mvwprintw(window, y, x + 2, "%8d", 0);
		mvwprintw(window, y + 1, x + 2, "%8d", 0);
		mvwprintw(window, y + 2, x + 2, "%8d", 0);
		mvwprintw(window, y + 3, x + 2, "%8d", 0);
		break;
	case 7:
		mvwprintw(window, y, x, "%12d", 0);
		mvwprintw(window, y + 1, x, "%12d", 0);
		mvwprintw(window, y + 2, x + 4, "%4d", 0);
		mvwprintw(window, y + 3, x + 4, "%4d", 0);
		break;
	default:
		mvwprintw(window, y, x, "%12d", 0);
		mvwprintw(window, y + 1, x, "%12d", 0);
		mvwprintw(window, y + 2, x, "%12d", 0);
		mvwprintw(window, y + 3, x, "%12d", 0);
		break;
	}
	wattroff(window, COLOR_PAIR(piece));
}

void print_piece_queue(WINDOW* window, int queue[PIECE_QUEUE_SIZE])
{
	for(int i = 0; i < PIECE_QUEUE_SIZE; i++)
	{
		print_piece(window, 4, 4 + 6 * i, queue[i]);
	}
}

void print_game_over(WINDOW* window, int** board)
{
	for(int i = BOARD_HEIGHT - 1; i >= 0; i--)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			board[i][j] = 0;
		}
		print_board(window, board);
		wrefresh(window);
		usleep(100 * 1000);
	}
	wattron(window, COLOR_PAIR(8));

	// G
	mvwprintw(window, 10, 4, "      ");
	mvwprintw(window, 11, 4, "  ");
	mvwprintw(window, 12, 4, "  ");
	mvwprintw(window, 12, 8, "  ");
	mvwprintw(window, 13, 4, "  ");
	mvwprintw(window, 13, 8, "  ");
	mvwprintw(window, 14, 4, "      ");
	// A
	mvwprintw(window, 10, 12, "      ");
	mvwprintw(window, 11, 12, "  ");
	mvwprintw(window, 11, 16, "  ");
	mvwprintw(window, 12, 12, "      ");
	mvwprintw(window, 13, 12, "  ");
	mvwprintw(window, 13, 16, "  ");
	mvwprintw(window, 14, 12, "  ");
	mvwprintw(window, 14, 16, "  ");
	// M
	mvwprintw(window, 10, 20, "          ");
	mvwprintw(window, 11, 20, "  ");
	mvwprintw(window, 11, 24, "  ");
	mvwprintw(window, 11, 28, "  ");
	mvwprintw(window, 12, 20, "  ");
	mvwprintw(window, 12, 24, "  ");
	mvwprintw(window, 12, 28, "  ");
	mvwprintw(window, 13, 20, "  ");
	mvwprintw(window, 13, 24, "  ");
	mvwprintw(window, 13, 28, "  ");
	mvwprintw(window, 14, 20, "  ");
	mvwprintw(window, 14, 24, "  ");
	mvwprintw(window, 14, 28, "  ");
	// E
	mvwprintw(window, 10, 32, "      ");
	mvwprintw(window, 11, 32, "  ");
	mvwprintw(window, 12, 32, "      ");
	mvwprintw(window, 13, 32, "  ");
	mvwprintw(window, 14, 32, "      ");
	// O
	mvwprintw(window, 16, 6, "      ");
	mvwprintw(window, 17, 6, "  ");
	mvwprintw(window, 17, 10, "  ");
	mvwprintw(window, 18, 6, "  ");
	mvwprintw(window, 18, 10, "  ");
	mvwprintw(window, 19, 6, "  ");
	mvwprintw(window, 19, 10, "  ");
	mvwprintw(window, 20, 6, "      ");
	// V
	mvwprintw(window, 16, 14, "  ");
	mvwprintw(window, 16, 18, "  ");
	mvwprintw(window, 17, 14, "  ");
	mvwprintw(window, 17, 18, "  ");
	mvwprintw(window, 18, 14, "  ");
	mvwprintw(window, 18, 18, "  ");
	mvwprintw(window, 19, 14, "  ");
	mvwprintw(window, 19, 18, "  ");
	mvwprintw(window, 20, 16, "  ");
	// E
	mvwprintw(window, 16, 22, "      ");
	mvwprintw(window, 17, 22, "  ");
	mvwprintw(window, 18, 22, "      ");
	mvwprintw(window, 19, 22, "  ");
	mvwprintw(window, 20, 22, "      ");
	// R
	mvwprintw(window, 16, 30, "      ");
	mvwprintw(window, 17, 30, "  ");
	mvwprintw(window, 17, 34, "  ");
	mvwprintw(window, 18, 30, "    ");
	mvwprintw(window, 19, 30, "  ");
	mvwprintw(window, 19, 34, "  ");
	mvwprintw(window, 20, 30, "  ");
	mvwprintw(window, 20, 34, "  ");

	wattroff(window, COLOR_PAIR(8));
	wrefresh(window);

	sleep(2);
}

void print_win(WINDOW* window, int** board, bool with_animation)
{
	for(int i = BOARD_HEIGHT - 1; i >= 0; i--)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			board[i][j] = 0;
		}
		if(with_animation)
		{
			print_board(window, board);
			wrefresh(window);
			usleep(100 * 1000);
		}
	}

	if(!with_animation)
	{
		print_board(window, board);
	}

	wattron(window, COLOR_PAIR(8));
	// W
	mvwprintw(window, 10, 8, "  ");
	mvwprintw(window, 10, 12, "  ");
	mvwprintw(window, 10, 16, "  ");
	mvwprintw(window, 11, 8, "  ");
	mvwprintw(window, 11, 12, "  ");
	mvwprintw(window, 11, 16, "  ");
	mvwprintw(window, 12, 8, "  ");
	mvwprintw(window, 12, 12, "  ");
	mvwprintw(window, 12, 16, "  ");
	mvwprintw(window, 13, 8, "  ");
	mvwprintw(window, 13, 12, "  ");
	mvwprintw(window, 13, 16, "  ");
	mvwprintw(window, 14, 8, "          ");
	// I
	mvwprintw(window, 10, 20, "  ");
	mvwprintw(window, 12, 20, "  ");
	mvwprintw(window, 13, 20, "  ");
	mvwprintw(window, 14, 20, "  ");
	// N
	mvwprintw(window, 10, 24, "  ");
	mvwprintw(window, 10, 32, "  ");
	mvwprintw(window, 11, 24, "    ");
	mvwprintw(window, 11, 32, "  ");
	mvwprintw(window, 12, 24, "  ");
	mvwprintw(window, 12, 28, "  ");
	mvwprintw(window, 12, 32, "  ");
	mvwprintw(window, 13, 24, "  ");
	mvwprintw(window, 13, 30, "    ");
	mvwprintw(window, 14, 24, "  ");
	mvwprintw(window, 14, 32, "  ");

	wattroff(window, COLOR_PAIR(8));
	wrefresh(window);
}

void print_loss(WINDOW* window, int** board, bool with_animation)
{
	for(int i = BOARD_HEIGHT - 1; i >= 0; i--)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			board[i][j] = 0;
		}
		if(with_animation)
		{
			print_board(window, board);
			wrefresh(window);
			usleep(100 * 1000);
		}
	}

	if(!with_animation)
	{
		print_board(window, board);
	}

	wattron(window, COLOR_PAIR(8));
	// L
	mvwprintw(window, 10, 6, "  ");
	mvwprintw(window, 11, 6, "  ");
	mvwprintw(window, 12, 6, "  ");
	mvwprintw(window, 13, 6, "  ");
	mvwprintw(window, 14, 6, "      ");
	// O
	mvwprintw(window, 10, 14, "      ");
	mvwprintw(window, 11, 14, "  ");
	mvwprintw(window, 11, 18, "  ");
	mvwprintw(window, 12, 14, "  ");
	mvwprintw(window, 12, 18, "  ");
	mvwprintw(window, 13, 14, "  ");
	mvwprintw(window, 13, 18, "  ");
	mvwprintw(window, 14, 14, "      ");
	// S
	mvwprintw(window, 10, 22, "      ");
	mvwprintw(window, 11, 22, "  ");
	mvwprintw(window, 12, 22, "      ");
	mvwprintw(window, 13, 26, "  ");
	mvwprintw(window, 14, 22, "      ");
	// E
	mvwprintw(window, 10, 30, "      ");
	mvwprintw(window, 11, 30, "  ");
	mvwprintw(window, 12, 30, "      ");
	mvwprintw(window, 13, 30, "  ");
	mvwprintw(window, 14, 30, "      ");

	wattroff(window, COLOR_PAIR(8));
	wrefresh(window);
}

void setup_ui()
{
	initscr();
	noecho();
	// cbreak();
	keypad(stdscr, TRUE);

	halfdelay(1); // tenths of a second
	curs_set(0);

	if(has_colors() == FALSE)
	{
		endwin();
		printf("Terminal does not support colors\n");
		exit(1);
	}

	start_color();

	init_pair(1, COLOR_BLACK, COLOR_BLACK); // Background
	// Piece colors
	init_pair(2, COLOR_RED, COLOR_RED); // I Piece
	init_pair(3, COLOR_YELLOW, COLOR_YELLOW); // L Piece
	init_pair(4, COLOR_BLUE, COLOR_BLUE); // J Piece
	init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA); // S Piece
	init_pair(6, COLOR_GREEN, COLOR_GREEN); // Z Piece
	init_pair(7, COLOR_YELLOW, COLOR_YELLOW); // O Piece
	init_pair(8, COLOR_WHITE, COLOR_WHITE); // T Piece
	// Text colors
	init_pair(DEFAULT_TEXT_COLOR, COLOR_WHITE, COLOR_BLACK);
	init_pair(10, COLOR_RED, COLOR_BLACK);
	init_pair(11, COLOR_GREEN, COLOR_BLACK);
	init_pair(12, COLOR_BLACK, COLOR_BLACK);

	refresh();
}

MENU* create_menu(char* choices[N_MENU_ITEMS])
{
	ITEM** menu_items;
	MENU* menu;

	menu_items = (ITEM**)calloc(N_MENU_ITEMS + 1, sizeof(ITEM*));
	for(int i = 0; i < N_MENU_ITEMS; i++)
	{
		menu_items[i] = new_item(choices[i], NULL);
	}
	menu_items[N_MENU_ITEMS] = (ITEM*)NULL;
	menu = new_menu(menu_items);
	return menu;
}

WINDOW* new_menu_window(MENU* menu)
{
	WINDOW* menu_window;
	menu_window = newwin(11, 4 * BOARD_WIDTH + 2 * BOARD_BORDER, 3, LEFT_PADDING);
	keypad(menu_window, TRUE);

	set_menu_win(menu, menu_window);
	set_menu_sub(menu, derwin(menu_window, 6, 38, 3, 1));
	set_menu_mark(menu, " >> ");
	box(menu_window, 0, 0);
	mvwprintw(menu_window, 1, 5, "%s", "Tetris");
	mvwhline(menu_window, 2, 1, ACS_HLINE, 4 * BOARD_WIDTH + 2 * BOARD_BORDER);
	mvwaddch(menu_window, 2, 0, ACS_LTEE);
	mvwaddch(menu_window, 2, 4 * BOARD_WIDTH + 2 * BOARD_BORDER - 1, ACS_RTEE);

	post_menu(menu);
	wrefresh(menu_window);

	return menu_window;
}

WINDOW* new_game_window(int left_offset)
{
	WINDOW* window = newwin(2 * BOARD_HEIGHT + 2 * BOARD_BORDER,
							4 * BOARD_WIDTH + 2 * BOARD_BORDER,
							3,
							LEFT_PADDING + left_offset);
	keypad(window, TRUE);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	wrefresh(window);

	return window;
}

WINDOW* new_header_window()
{
	WINDOW* window = newwin(2, COLS, 0, 0);
	keypad(window, TRUE);

	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	print_header(window);
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	wrefresh(window);

	return window;
}

WINDOW* new_hold_piece_window()
{
	WINDOW* window = newwin(10, 20, 35, 4 * BOARD_WIDTH + 2 * BOARD_BORDER + LEFT_PADDING * 2);
	keypad(window, TRUE);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	mvwhline(window, 2, 0, ACS_HLINE, 20);
	mvwaddch(window, 2, 0, ACS_LTEE);
	mvwaddch(window, 2, 19, ACS_RTEE);
	mvwprintw(window, 1, 2, "Hold");
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	wrefresh(window);

	return window;
}

WINDOW* new_score_window(int left_offset)
{
	WINDOW* window =
		newwin(5, 20, 3, 4 * BOARD_WIDTH + 2 * BOARD_BORDER + LEFT_PADDING * 2 + left_offset);
	keypad(window, TRUE);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	mvwhline(window, 2, 0, ACS_HLINE, 20);
	mvwaddch(window, 2, 0, ACS_LTEE);
	mvwaddch(window, 2, 19, ACS_RTEE);
	mvwprintw(window, 1, 2, "Score");
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	wrefresh(window);

	return window;
}

WINDOW* new_piece_queue_window()
{
	WINDOW* window = newwin(22, 20, 8, 4 * BOARD_WIDTH + 2 * BOARD_BORDER + LEFT_PADDING * 2);
	keypad(window, TRUE);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	mvwhline(window, 2, 0, ACS_HLINE, 20);
	mvwaddch(window, 2, 0, ACS_LTEE);
	mvwaddch(window, 2, 19, ACS_RTEE);
	mvwprintw(window, 1, 2, "Next");
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	wrefresh(window);

	return window;
}

WINDOW* new_leaderboard_window()
{
	WINDOW* window = newwin(18, 4 * BOARD_WIDTH + 2 * BOARD_BORDER, 3, LEFT_PADDING);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	mvwhline(window, 2, 0, ACS_HLINE, 4 * BOARD_WIDTH + 2 * BOARD_BORDER);
	mvwaddch(window, 2, 0, ACS_LTEE);
	mvwaddch(window, 2, 2 * BOARD_BORDER + 4 * BOARD_WIDTH - 1, ACS_RTEE);
	mvwprintw(window, 1, 2, "Leaderboard");
	mvwprintw(window, 3, 6, "Name");
	mvwprintw(window, 3, 27, "Score");
	mvwprintw(window, 16, 2, "press q to return");
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	wrefresh(window);

	return window;
}

WINDOW* new_controls_window()
{
	WINDOW* window = newwin(13, 6 * BOARD_WIDTH + 2 * BOARD_BORDER, 3, LEFT_PADDING);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	mvwhline(window, 2, 0, ACS_HLINE, 6 * BOARD_WIDTH + 2 * BOARD_BORDER);
	mvwaddch(window, 2, 0, ACS_LTEE);
	mvwaddch(window, 2, 2 * BOARD_BORDER + 6 * BOARD_WIDTH - 1, ACS_RTEE);
	mvwprintw(window, 1, 2, "Controls");
	mvwprintw(window, 3, 6, "Left Arrow");
	mvwprintw(window, 3, 19, "- Move piece left");
	mvwprintw(window, 4, 6, "Right Arrow");
	mvwprintw(window, 4, 19, "- Move piece right");
	mvwprintw(window, 5, 6, "Down Arrow");
	mvwprintw(window, 5, 19, "- Move piece down");
	mvwprintw(window, 6, 6, "Space");
	mvwprintw(window, 6, 19, "- Drop piece");
	mvwprintw(window, 7, 6, "Q");
	mvwprintw(window, 7, 19, "- Rotate piece counter clockwise");
	mvwprintw(window, 8, 6, "E");
	mvwprintw(window, 8, 19, "- Rotate piece clockwise");
	mvwprintw(window, 9, 6, "W");
	mvwprintw(window, 9, 19, "- Hold piece");
	mvwprintw(window, 11, 6, "press q to return");
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	wrefresh(window);

	return window;
}

void print_highscore(WINDOW* window)
{
	wattron(window, COLOR_PAIR(9));
	wattroff(window, COLOR_PAIR(9));
}

FORM* create_highscore_form(FIELD* fields[N_FIELDS])
{
	FORM* form = new_form(fields);

	set_field_buffer(fields[1], 0, "");

	set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
	set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_back(fields[1], A_UNDERLINE);

	set_field_type(fields[1], TYPE_ALPHA, 1);

	return form;
}

WINDOW* new_highscore_window(FORM* form)
{
	WINDOW* window = newwin(5, 4 * BOARD_WIDTH + 2 * BOARD_BORDER, 3 + 18, LEFT_PADDING);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	mvwhline(window, 2, 0, ACS_HLINE, 4 * BOARD_WIDTH + 2 * BOARD_BORDER);
	mvwaddch(window, 2, 0, ACS_LTEE);
	mvwaddch(window, 2, 2 * BOARD_BORDER + 4 * BOARD_WIDTH - 1, ACS_RTEE);
	mvwprintw(window, 1, 2, "New Highscore!");
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));

	set_form_win(form, window);
	set_form_sub(form, derwin(window, 1, 40, 3, 1));
	post_form(form);

	wrefresh(window);
	return window;
}

WINDOW* new_queue_window()
{
	WINDOW* window = newwin(7, 4 * BOARD_WIDTH + 2 * BOARD_BORDER, 3, LEFT_PADDING);
	wattron(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));
	box(window, 0, 0);
	mvwhline(window, 2, 0, ACS_HLINE, 4 * BOARD_WIDTH + 2 * BOARD_BORDER);
	mvwaddch(window, 2, 0, ACS_LTEE);
	mvwaddch(window, 2, 2 * BOARD_BORDER + 4 * BOARD_WIDTH - 1, ACS_RTEE);
	mvwprintw(window, 1, 2, "Multiplayer");
	mvwprintw(window, 3, 2, "Searching for opponent...");
	mvwprintw(window, 5, 2, "press q to cancel.");
	wattroff(window, COLOR_PAIR(DEFAULT_TEXT_COLOR));

	wrefresh(window);
	return window;
}

void cleanup_menu(WINDOW* menu_window, MENU* menu, ITEM** menu_items, int n_menu_items)
{
	unpost_menu(menu);
	for(int i = 0; i < n_menu_items; i++)
	{
		free(menu_items[i]);
	}
	free_menu(menu);
	werase(menu_window);
}

void refresh_ui(struct App* app, WINDOW* windows[N_WINDOWS])
{
	if(app->connection->awaiting_connect_response)
	{
		print_connection_status(windows[HEADER_WINDOW], CONNECTING);
	}
	else if(app->connection->open)
	{
		print_connection_status(windows[HEADER_WINDOW], CONNECTED);
	}
	else
	{
		print_connection_status(windows[HEADER_WINDOW], DISCONNECTED);
	}

	print_ping(windows[HEADER_WINDOW], app->connection->ping);
	wrefresh(windows[HEADER_WINDOW]);
	wrefresh(windows[MENU_WINDOW]);

	if(app->game_states[app->player]->playing)
	{
		print_board(windows[GAME_WINDOW], app->game_states[app->player]->board);
		print_score(windows[SCORE_WINDOW], app->game_states[app->player]->score);
		print_piece_queue(windows[PIECE_QUEUE_WINDOW], app->game_states[app->player]->piece_queue);
		print_piece(windows[HOLD_PIECE_WINDOW], 4, 4, app->game_states[app->player]->hold_piece);
		wrefresh(windows[GAME_WINDOW]);
		wrefresh(windows[SCORE_WINDOW]);
		wrefresh(windows[PIECE_QUEUE_WINDOW]);
		wrefresh(windows[HOLD_PIECE_WINDOW]);
	}

	int opponent = 1 - app->player;
	if(app->game_mode == MULTIPLAYER && app->game_states[opponent]->playing)
	{
		print_board(windows[OPPONENT_WINDOW], app->game_states[opponent]->board);
		print_score(windows[OPPONENT_SCORE_WINDOW], app->game_states[opponent]->score);
		wrefresh(windows[OPPONENT_WINDOW]);
		wrefresh(windows[OPPONENT_SCORE_WINDOW]);
	}
}

void driver_form(FORM* form, int ch, int name_length)
{
	switch(ch)
	{
	case KEY_LEFT:
		form_driver(form, REQ_PREV_CHAR);
		break;

	case KEY_RIGHT:
		form_driver(form, REQ_NEXT_CHAR);
		break;

	case KEY_BACKSPACE:
	case 127:
		form_driver(form, REQ_DEL_PREV);
		break;

	case KEY_DC:
		form_driver(form, REQ_DEL_CHAR);
		break;

	default:
		if(name_length < 19)
		{
			form_driver(form, ch);
		}
		break;
	}
}

void create_windows(WINDOW* windows[N_WINDOWS], PANEL* panels[N_WINDOWS], MENU* menu, FORM* form)
{
	windows[MENU_WINDOW] = new_menu_window(menu);
	windows[GAME_WINDOW] = new_game_window(0);
	windows[OPPONENT_WINDOW] = new_game_window(102);
	windows[HEADER_WINDOW] = new_header_window();
	windows[SCORE_WINDOW] = new_score_window(0);
	windows[OPPONENT_SCORE_WINDOW] = new_score_window(30);
	windows[LEADERBOARD_WINDOW] = new_leaderboard_window();
	windows[HIGHSCORE_WINDOW] = new_highscore_window(form);
	windows[QUEUE_WINDOW] = new_queue_window();
	windows[PIECE_QUEUE_WINDOW] = new_piece_queue_window();
	windows[HOLD_PIECE_WINDOW] = new_hold_piece_window();
	windows[CONTROLS_WINDOW] = new_controls_window();

	panels[MENU_WINDOW] = new_panel(windows[MENU_WINDOW]);
	panels[GAME_WINDOW] = new_panel(windows[GAME_WINDOW]);
	panels[OPPONENT_WINDOW] = new_panel(windows[OPPONENT_WINDOW]);
	panels[HEADER_WINDOW] = new_panel(windows[HEADER_WINDOW]);
	panels[SCORE_WINDOW] = new_panel(windows[SCORE_WINDOW]);
	panels[OPPONENT_SCORE_WINDOW] = new_panel(windows[OPPONENT_SCORE_WINDOW]);
	panels[LEADERBOARD_WINDOW] = new_panel(windows[LEADERBOARD_WINDOW]);
	panels[HIGHSCORE_WINDOW] = new_panel(windows[HIGHSCORE_WINDOW]);
	panels[QUEUE_WINDOW] = new_panel(windows[QUEUE_WINDOW]);
	panels[PIECE_QUEUE_WINDOW] = new_panel(windows[PIECE_QUEUE_WINDOW]);
	panels[HOLD_PIECE_WINDOW] = new_panel(windows[HOLD_PIECE_WINDOW]);
	panels[CONTROLS_WINDOW] = new_panel(windows[CONTROLS_WINDOW]);
}
