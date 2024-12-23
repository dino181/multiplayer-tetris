#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "constants.h"

#define TIMEOUT 1 // sec
#define MESSAGE_TIMEOUT 5

void update_scores(struct HighScore** scores, char new_scores[1024])
{
	char* name = strtok(new_scores, ",");
	int score = atoi(strtok(NULL, ";"));

	strcpy(scores[0]->name, name);
	scores[0]->score = score;

	for(int i = 1; i < 10; i++)
	{
		name = strtok(NULL, ",");
		score = atoi(strtok(NULL, ";"));
		strcpy(scores[i]->name, name);
		scores[i]->score = score;
	}
}

void send_message(struct Connection* connection, char* message)
{
	socklen_t address_length = sizeof(connection->server);
	sendto(connection->client,
		   message,
		   strlen(message),
		   0,
		   (struct sockaddr*)&(connection->server),
		   address_length);
}

void send_highscore_name(struct Connection* connection, char* name)
{
	char message[1024] = {0};
	sprintf(message, "HIGHSCORE %s", name);
	send_message(connection, message);
}

void request_leaderboard(struct Connection* connection)
{
	send_message(connection, "SCORES");
}

void send_connect(struct Connection* connection)
{
	send_message(connection, "CONNECT");
	connection->awaiting_connect_response = 1;
}

void send_start(struct Connection* connection)
{
	send_message(connection, "START");
	connection->awaiting_start_response = 1;
}

void send_action(struct Connection* connection, char action)
{
	char message[9] = "ACTION -\0";
	message[7] = action;

	send_message(connection, message);
}

void send_queue(struct App* app, bool enter_queue)
{
	if(enter_queue)
	{
		send_message(app->connection, "QUEUE ENTER");
		app->in_queue = true;
	}
	else
	{
		send_message(app->connection, "QUEUE CANCEL");
		app->in_queue = false;
	}
}

void update_game_state(struct GameState* client_state, char server_state[1024])
{
	char* board_buf = strtok(server_state, "_");

	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			client_state->board[i][j] = board_buf[2 * BOARD_WIDTH * i + 2 * j] - '0';
		}
	}

	char* game_over = strtok(NULL, "_");
	client_state->game_over = atoi(game_over) == 1;

	char* score = strtok(NULL, "_");
	client_state->score = atoi(score);

	char* winning = strtok(NULL, "_");
	client_state->winning = atoi(winning) == 1;

	char* piece_queue = strtok(NULL, "_");
	client_state->piece_queue[0] = piece_queue[0] - '0';
	client_state->piece_queue[1] = piece_queue[2] - '0';
	client_state->piece_queue[2] = piece_queue[4] - '0';

	char* hold_piece = strtok(NULL, "_");
	client_state->hold_piece = atoi(hold_piece);
}

void* listen_on_connection(void* args)
{
	struct App* app = args;

	ssize_t response_length;

	int n_failed_responses = 0;
	int ping = 0;
	int ping_counter = 0;
	int connect_counter = 0;
	int start_counter = 0;
	char ping_return[8] = "PING OK\0";

	while(app->running)
	{
		char buf[1024] = {0};
		response_length = recv(app->connection->client, buf, 1024, 0);

		if(response_length == -1)
		{
			if(app->connection->awaiting_connect_response)
			{
				connect_counter++;

				if(connect_counter >= MESSAGE_TIMEOUT)
				{
					app->connection->open = false;
					app->connection->awaiting_connect_response = 0;
				}
			}

			else if(app->connection->awaiting_start_response)
			{
				start_counter++;
				if(start_counter >= MESSAGE_TIMEOUT)
				{
					app->connection->open = false;
					app->connection->awaiting_start_response = 0;
				}
			}

			else
			{
				ping_counter++;
				if(ping_counter >= MESSAGE_TIMEOUT)
				{
					app->connection->open = false;
				}
			}

			continue;
		}

		char* identifier = strtok(buf, " ");

		if(strcmp(identifier, "STATE") == 0)
		{
			char* mode = strtok(NULL, " ");
			int multiplayer = strcmp(mode, "MP") == 0;

			char* player_states[2] = {strtok(NULL, " "), strtok(NULL, " ")};

			update_game_state(app->game_states[app->player], player_states[app->player]);

			int opponent = 1 - app->player;
			if(multiplayer && player_states[opponent] != NULL)
			{
				update_game_state(app->game_states[opponent], player_states[opponent]);
			}
		}

		else if(strcmp(identifier, "PING") == 0)
		{
			ping = atoi(strtok(NULL, " "));
			app->connection->ping = ping;
			sendto(app->connection->client,
				   ping_return,
				   strlen(ping_return),
				   0,
				   (struct sockaddr*)&(app->connection->server),
				   sizeof(app->connection->server));
			ping_counter = 0;
		}

		else if(strcmp(identifier, "START") == 0)
		{
			char* start_result = strtok(NULL, " ");
			if(strcmp(start_result, "OK") == 0)
			{
				app->player = 0;
				app->game_states[0]->playing = true;
			}
			else
			{
				app->connection->open = false;
			}
			app->connection->awaiting_start_response = false;
		}

		else if(strcmp(identifier, "MATCH") == 0)
		{
			char* queue_result = strtok(NULL, " ");
			if(strcmp(queue_result, "P1") == 0 && app->in_queue)
			{
				app->player = 0;
				app->in_queue = false;
			}
			else if(strcmp(queue_result, "P2") == 0 && app->in_queue)
			{
				app->player = 1;
				app->in_queue = false;
			}
		}

		else if(strcmp(identifier, "CONNECT") == 0)
		{
			char* connect_result = strtok(NULL, " ");
			if(strcmp(connect_result, "OK") == 0)
			{
				app->connection->open = true;
			}
			else
			{
				app->connection->open = false;
			}
			app->connection->awaiting_connect_response = false;
		}

		else if(strcmp(identifier, "NOCONN") == 0)
		{
			app->connection->open = false;
		}

		else if(strcmp(identifier, "SCORES") == 0)
		{
			char* new_scores = strtok(NULL, "\n");
			update_scores(app->scores, new_scores);
		}
	}

	return 0;
}

int open_socket()
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(sock == -1)
	{
		printf("Failed to create socket.");
		exit(EXIT_FAILURE);
	}

	struct timeval time_val;
	time_val.tv_sec = 1;
	time_val.tv_usec = 0;

	int success =
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time_val, sizeof(time_val));
	if(success == -1)
	{
		printf("Failed to set timeout for socket. Errno: %d", errno);
		exit(EXIT_FAILURE);
	}

	return sock;
}

struct sockaddr_in get_server_address()
{
	struct sockaddr_in server_address;

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);

	int success = inet_pton(AF_INET, SERVER_ADDRESS, &server_address.sin_addr);
	if(success != 1)
	{
		printf("Failed to create server address");
		exit(EXIT_FAILURE);
	}

	return server_address;
}

struct Connection* get_connection()
{
	struct Connection* connection = (struct Connection*)malloc(sizeof(struct Connection));

	connection->open = true;
	connection->client = open_socket();
	connection->server = get_server_address();
	connection->ping = 0;
	connection->awaiting_connect_response = false;
	connection->awaiting_start_response = false;

	return connection;
}

void close_connection(struct Connection* connection, pthread_t thread_id)
{
	connection->open = false;
	pthread_join(thread_id, NULL);
	close(connection->client);
}
