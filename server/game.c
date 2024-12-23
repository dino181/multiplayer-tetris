#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "game.h"
#include "movements.h"
#include "pieces.h"
#include "server.h"
#include "types.h"

#define CYCLE_TIME 25 // ms

int** create_board()
{
	int** board = (int**)malloc(BOARD_HEIGHT * sizeof(int*));
	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		board[i] = (int*)malloc(BOARD_WIDTH * sizeof(int));
	};
	return board;
}

struct SingleplayerGame* initialize_game(int server, struct Client* client)
{
	struct SingleplayerGame* game =
		(struct SingleplayerGame*)malloc(sizeof(struct SingleplayerGame));

	game->client = client;
	game->server = server;

	return game;
}

struct MultiplayerGame* initialize_multiplayer_game(int server, struct Client* player1,
													struct Client* player2)
{
	struct MultiplayerGame* game = (struct MultiplayerGame*)malloc(sizeof(struct MultiplayerGame));

	game->players[0] = player1;
	game->players[1] = player2;
	game->server = server;

	return game;
}

int calculate_score(int level, int lines_cleared)
{
	switch(lines_cleared)
	{
	case 1:
		return 40 * level;
	case 2:
		return 100 * level;
	case 3:
		return 300 * level;
	case 4:
		return 1200 * level;
	default:
		return 0;
	}
}

int clear_lines(int** board)
{
	int cleared_lines[4] = {-1, -1, -1, -1};
	int n_cleared_lines = 0;

	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		int line_cleared = 1;
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			if(board[i][j] == 0)
			{
				line_cleared = 0;
				break;
			}
		}
		if(line_cleared)
		{
			cleared_lines[n_cleared_lines] = i;
			n_cleared_lines++;
		}
	}

	for(int i = 0; i < n_cleared_lines; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			board[cleared_lines[i]][j] = 0;
		}

		int* current_line = board[cleared_lines[i]];
		for(int j = cleared_lines[i]; j > 0; j--)
		{
			board[j] = board[j - 1];
		}

		board[0] = current_line;
	}

	return n_cleared_lines;
}

int handle_action(struct Client* client, struct Piece* piece)
{
	int can_move_down = 1;
	int held_piece = 0;
	switch(client->game_state.current_action)
	{
	case 'l':
		move_left(client->game_state.board, piece);
		break;
	case 'r':
		move_right(client->game_state.board, piece);
		break;
	case 'd':
		can_move_down = move_down(client->game_state.board, piece);
		break;
	case 's':
		while(can_move_down)
		{
			can_move_down = move_down(client->game_state.board, piece);
		}
		break;
	case 'e':
		rotate_cw(client->game_state.board, piece);
		break;
	case 'q':
		rotate_ccw(client->game_state.board, piece);
		break;
	case 'h':
		if(!client->game_state.can_hold)
		{
			break;
		}

		clear_piece(client->game_state.board, piece);
		int current_held_piece = client->game_state.hold_piece;
		client->game_state.held_piece = current_held_piece;
		client->game_state.hold_piece = piece->value;

		client->game_state.use_held_piece = current_held_piece != 0;
		client->game_state.can_hold = false;
		can_move_down = 0;
		break;
	default:
		break;
	}

	return can_move_down;
}

void* run_singleplayer_game(void* args)
{
	struct SingleplayerGame* game = args;

	unsigned long int queue_seed = 0;
	unsigned long int base_seed = game->client->game_thread;

	int current_piece_index = 0;
	int piece_queue[2 * N_PIECES] = {1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7};
	update_piece_queue(piece_queue, 0, base_seed + queue_seed);
	queue_seed++;
	update_piece_queue(piece_queue, 1, base_seed + queue_seed);
	queue_seed++;

	struct Piece* piece = generate_piece(piece_queue[current_piece_index]);
	current_piece_index++;
	game->client->game_state.piece_queue[0] = piece_queue[(current_piece_index) % (2 * N_PIECES)];
	game->client->game_state.piece_queue[1] =
		piece_queue[(current_piece_index + 1) % (2 * N_PIECES)];
	game->client->game_state.piece_queue[2] =
		piece_queue[(current_piece_index + 2) % (2 * N_PIECES)];

	int drop_speed = 2; // updates per second
	int active_piece = 1;
	int can_move_down = 1;
	int total_cleared_lines = 0;

	struct timeval start, end;
	gettimeofday(&start, NULL);

	free(game->client->game_state.board);
	game->client->game_state.board = create_board();
	game->client->game_state.score = 0;
	game->client->game_state.game_over = 0;
	game->client->game_state.current_action = NO_INPUT_CHAR;
	game->client->saving_score = 0;

	while(!(game->client->game_state.game_over))
	{
		if(!(game->client->has_connection))
		{
			printf("[SERVER] No connection to client....\n");
			break;
		}

		if(!active_piece)
		{
			free(piece);
			if(game->client->game_state.use_held_piece)
			{
				piece = generate_piece(game->client->game_state.held_piece);
				game->client->game_state.use_held_piece = false;
			}
			else
			{
				game->client->game_state.can_hold = true;
				piece = generate_piece(piece_queue[current_piece_index]);
				current_piece_index = (current_piece_index + 1) % (2 * N_PIECES);

				game->client->game_state.piece_queue[0] =
					piece_queue[(current_piece_index) % (2 * N_PIECES)];
				game->client->game_state.piece_queue[1] =
					piece_queue[(current_piece_index + 1) % (2 * N_PIECES)];
				game->client->game_state.piece_queue[2] =
					piece_queue[(current_piece_index + 2) % (2 * N_PIECES)];

				if(current_piece_index == 1 || current_piece_index == 8)
				{
					update_piece_queue(
						piece_queue, current_piece_index == 1, base_seed + queue_seed);
					queue_seed++;
				}
			}

			for(int i = 0; i < piece->size; i++)
			{
				int x = piece->squares[i][0] + piece->center_position[0];
				int y = piece->squares[i][1] + piece->center_position[1];
				if(game->client->game_state.board[y][x] != 0)
				{
					game->client->game_state.game_over = 1;
				}
				game->client->game_state.board[y][x] = piece->value;
			}
			active_piece = 1;

			if(game->client->game_state.game_over)
			{
				send_singleplayer_game_state(game->server, game->client);
				break;
			}
		}

		can_move_down = handle_action(game->client, piece);

		game->client->game_state.current_action = NO_INPUT_CHAR;

		gettimeofday(&end, NULL);
		long time_diff =
			((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000.0) + 0.5;

		if(can_move_down && time_diff >= 1000 / game->client->game_state.level * drop_speed)
		{
			can_move_down = move_down(game->client->game_state.board, piece);
			gettimeofday(&start, NULL);
		}

		if(!can_move_down)
		{
			active_piece = 0;
			can_move_down = 1;
			int lines_cleared = clear_lines(game->client->game_state.board);
			total_cleared_lines += lines_cleared;
			game->client->game_state.level = 1 + total_cleared_lines / 10;
			game->client->game_state.score +=
				calculate_score(game->client->game_state.level, lines_cleared);
		}

		send_singleplayer_game_state(game->server, game->client);
		usleep(CYCLE_TIME * 1000);
	}

	game->client->game_state.playing = 0;
	game->client->saving_score = 1;
	game->client->close_thread = 1;

	printf("[SERVER] GAME OVER\n");

	free(game);
	free(piece);

	return 0;
}

void* run_multiplayer_game(void* args)
{
	struct MultiplayerGame* game = args;

	unsigned long int queue_seeds[2] = {0, 0};
	unsigned long int base_seed = game->players[0]->game_thread;

	printf("[SERVER] starting multiplayer game...\n");
	printf("[SERVER] player1: %d:%d\n",
		   game->players[0]->sock.sin_addr.s_addr,
		   game->players[0]->sock.sin_port);
	printf("[SERVER] player2: %d:%d\n",
		   game->players[1]->sock.sin_addr.s_addr,
		   game->players[1]->sock.sin_port);

	send_match_created(game->server, game->players[0]->sock, 1);
	send_match_created(game->server, game->players[1]->sock, 2);

	int current_piece_index[2] = {0, 0};
	int piece_queue[2][2 * N_PIECES] = {{1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7},
										{1, 2, 3, 4, 5, 6, 7, 1, 2, 3, 4, 5, 6, 7}};
	update_piece_queue(piece_queue[0], 0, base_seed + queue_seeds[0]);
	queue_seeds[0]++;
	update_piece_queue(piece_queue[0], 1, base_seed + queue_seeds[0]);
	queue_seeds[0]++;
	update_piece_queue(piece_queue[1], 0, base_seed + queue_seeds[1]);
	queue_seeds[1]++;
	update_piece_queue(piece_queue[1], 1, base_seed + queue_seeds[1]);
	queue_seeds[1]++;

	game->players[0]->game_state.piece_queue[0] =
		piece_queue[0][(current_piece_index[0]) % (2 * N_PIECES)];
	game->players[0]->game_state.piece_queue[1] =
		piece_queue[0][(current_piece_index[0] + 1) % (2 * N_PIECES)];
	game->players[0]->game_state.piece_queue[2] =
		piece_queue[0][(current_piece_index[0] + 2) % (2 * N_PIECES)];
	game->players[1]->game_state.piece_queue[0] =
		piece_queue[1][(current_piece_index[1]) % (2 * N_PIECES)];
	game->players[1]->game_state.piece_queue[1] =
		piece_queue[1][(current_piece_index[1] + 1) % (2 * N_PIECES)];
	game->players[1]->game_state.piece_queue[2] =
		piece_queue[1][(current_piece_index[1] + 2) % (2 * N_PIECES)];

	struct Piece* piece[2] = {generate_piece(piece_queue[0][current_piece_index[0]]),
							  generate_piece(piece_queue[1][current_piece_index[1]])};
	current_piece_index[0]++;
	current_piece_index[1]++;

	int drop_speed = 2; // updates per second
	int active_piece[2] = {1, 1}; // first is player1, second is player2
	int can_move_down[2] = {1, 1}; // first is player1, second is player2
	int total_lines_cleared[2] = {0, 0}; // first is player1, second is player2

	struct timeval start[2];
	struct timeval end;

	gettimeofday(&start[0], NULL);
	gettimeofday(&start[1], NULL);

	for(int i = 0; i < 2; i++)
	{
		free(game->players[i]->game_state.board);
		game->players[i]->game_state.board = create_board();
		game->players[i]->game_state.can_hold = true;
		game->players[i]->game_state.score = 0;
		game->players[i]->game_state.game_over = 0;
		game->players[i]->game_state.current_action = NO_INPUT_CHAR;
		game->players[i]->saving_score = 0;
	}

	while(true)
	{
		game->players[0]->game_state.winning =
			game->players[0]->game_state.score >= game->players[1]->game_state.score;
		game->players[1]->game_state.winning =
			game->players[1]->game_state.score >= game->players[0]->game_state.score;

		if(!(game->players[0]->has_connection) || !(game->players[1]->has_connection))
		{
			printf("[SERVER] No connection to client....\n");
			break;
		}

		if(game->players[0]->game_state.game_over && game->players[1]->game_state.game_over)
		{
			printf("[SERVER] Both players are game over\n");
			break;
		}

		for(int player = 0; player < 2; player++)
		{
			if(game->players[player]->game_state.game_over)
			{
				continue;
			}

			if(!active_piece[player])
			{
				free(piece[player]);
				if(game->players[player]->game_state.use_held_piece)
				{
					piece[player] = generate_piece(game->players[player]->game_state.held_piece);
					game->players[player]->game_state.use_held_piece = false;
				}
				else
				{
					game->players[player]->game_state.can_hold = true;
					piece[player] =
						generate_piece(piece_queue[player][current_piece_index[player]]);
					current_piece_index[player] =
						(current_piece_index[player] + 1) % (2 * N_PIECES);

					game->players[player]->game_state.piece_queue[0] =
						piece_queue[player][(current_piece_index[player]) % (2 * N_PIECES)];
					game->players[player]->game_state.piece_queue[1] =
						piece_queue[player][(current_piece_index[player] + 1) % (2 * N_PIECES)];
					game->players[player]->game_state.piece_queue[2] =
						piece_queue[player][(current_piece_index[player] + 2) % (2 * N_PIECES)];

					if(current_piece_index[player] == 1 || current_piece_index[player] == 8)
					{
						update_piece_queue(piece_queue[player],
										   current_piece_index[player] == 1,
										   queue_seeds[player]);
						queue_seeds[player]++;
					}
				}

				for(int i = 0; i < piece[player]->size; i++)
				{
					int x = piece[player]->squares[i][0] + piece[player]->center_position[0];
					int y = piece[player]->squares[i][1] + piece[player]->center_position[1];
					if(game->players[player]->game_state.board[y][x] != 0)
					{
						game->players[player]->game_state.game_over = 1;
					}
					game->players[player]->game_state.board[y][x] = piece[player]->value;
				}
				active_piece[player] = 1;

				if(game->players[player]->game_state.game_over == 1)
				{
					continue;
				}
			}

			can_move_down[player] = handle_action(game->players[player], piece[player]);
			game->players[player]->game_state.current_action = NO_INPUT_CHAR;

			gettimeofday(&end, NULL);
			long time_diff = ((end.tv_sec - start[player].tv_sec) * 1000 +
							  (end.tv_usec - start[player].tv_usec) / 1000.0) +
							 0.5;

			if(can_move_down[player] &&
			   time_diff >= 1000 / game->players[player]->game_state.level * drop_speed)
			{
				can_move_down[player] =
					move_down(game->players[player]->game_state.board, piece[player]);
				gettimeofday(&start[player], NULL);
			}

			if(!can_move_down[player])
			{
				active_piece[player] = 0;
				can_move_down[player] = 1;
				int lines_cleared = clear_lines(game->players[player]->game_state.board);
				total_lines_cleared[player] += lines_cleared;
				game->players[player]->game_state.level = 1 + total_lines_cleared[player] / 10;
				game->players[player]->game_state.score +=
					calculate_score(game->players[player]->game_state.level, lines_cleared);
			}
		}

		send_multiplayer_game_state(game->server, game->players);
		usleep(CYCLE_TIME * 1000);
	}

	game->players[0]->game_state.playing = 0;
	game->players[1]->game_state.playing = 0;
	game->players[0]->close_thread = 1;

	printf("[SERVER] MULTIPLAYER GAME OVER\n");

	free(game);
	free(piece[0]);
	free(piece[1]);

	return 0;
}
