#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "game.h"
#include "high_scores.h"
#include "queue.h"
#include "server.h"

#define PORT 8080
#define PING_TIMEOUT 2

void log_connection(struct sockaddr_in client, int n_clients, int is_connecting)
{
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char* time = asctime(timeinfo);
	if(time[strlen(time) - 1] == '\n')
	{
		time[strlen(time) - 1] = '\0';
	}

	FILE* fptr;
	fptr = fopen("connection_log.txt", "a");
	if(is_connecting)
	{
		fprintf(fptr,
				"[%s] Client connected with server: %s. Now serving (%d/%d)\n",
				time,
				inet_ntoa(client.sin_addr),
				n_clients,
				MAX_CLIENTS);
	}
	else
	{
		fprintf(fptr,
				"[%s] Client disconnected from server: %s. Now serving (%d/%d)\n",
				time,
				inet_ntoa(client.sin_addr),
				n_clients,
				MAX_CLIENTS);
	}

	fclose(fptr);
}

int is_connected(int address, int port, struct Client* clients[MAX_CLIENTS], int n_clients)
{
	for(int i = 0; i < n_clients; i++)
	{
		if(clients[i]->sock.sin_addr.s_addr == address && clients[i]->sock.sin_port == port)
		{
			return 1;
		}
	}
	return 0;
}

int open_socket()
{
	struct sockaddr_in address;
	int opt = 1;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock == -1)
	{
		printf("[SERVER] Failed to create server socket.\n");
		return 1;
	}

	int success = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

	if(success == -1)
	{
		printf("[SERVER] Failed to set socket options.\n");
		return 1;
	}

	struct timeval time_val;
	time_val.tv_sec = 1;
	time_val.tv_usec = 0;

	success = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time_val, sizeof(time_val));
	if(success == -1)
	{
		printf("[SERVER] Failed to set timeout for socket. Errno: %d", errno);
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	success = bind(sock, (struct sockaddr*)&address, sizeof(address));
	if(success == -1)
	{
		printf("[SERVER] Failed to bind to address.");
		return 1;
	}

	printf("[SERVER] Created socket. Waiting for a client to establish connection...\n");

	return sock;
}

int establish_new_connection(struct sockaddr_in client, struct Client* clients[MAX_CLIENTS],
							 int n_clients)
{
	if(!is_connected(client.sin_addr.s_addr, client.sin_port, clients, n_clients))
	{
		if(n_clients >= MAX_CLIENTS)
		{
			printf("[SERVER] The amount of connections exceeds server capacity. Not adding "
				   "connection\n");
			return -1;
		}
		clients[n_clients]->sock = client;
		clients[n_clients]->has_connection = 1;

		printf("[SERVER] New connection established. Currently serving (%d/%d) clients\n",
			   n_clients + 1,
			   MAX_CLIENTS);
		log_connection(client, n_clients + 1, 1);

		return n_clients + 1;
	}

	return n_clients;
}

void send_singleplayer_game_state(int sock, struct Client* client)
{
	char game_state[1024] = {0};
	char board_str[2 * BOARD_WIDTH * BOARD_HEIGHT + 1] = {0};
	socklen_t client_length = sizeof(client->sock);

	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			board_str[2 * BOARD_WIDTH * i + 2 * j] = client->game_state.board[i][j] + '0';
			board_str[2 * BOARD_WIDTH * i + 2 * j + 1] = ',';
		}
		board_str[2 * BOARD_WIDTH * (i + 1) - 1] = ';';
	}

	int queue[3] = {4, 1, 3};
	sprintf(game_state,
			"STATE SP %s_%d_%d_%d_%d,%d,%d_%d",
			board_str,
			client->game_state.game_over,
			client->game_state.score,
			client->game_state.winning,
			client->game_state.piece_queue[0],
			client->game_state.piece_queue[1],
			client->game_state.piece_queue[2],
			client->game_state.hold_piece);
	sendto(sock, game_state, strlen(game_state), 0, (struct sockaddr*)&client->sock, client_length);
}

void send_multiplayer_game_state(int sock, struct Client* players[2])
{
	char game_state[1024] = {0};
	char player1_board[2 * BOARD_WIDTH * BOARD_HEIGHT + 1] = {0};
	char player2_board[2 * BOARD_WIDTH * BOARD_HEIGHT + 1] = {0};
	socklen_t client_length[2] = {sizeof(players[0]->sock), sizeof(players[1]->sock)};

	for(int i = 0; i < BOARD_HEIGHT; i++)
	{
		for(int j = 0; j < BOARD_WIDTH; j++)
		{
			player1_board[2 * BOARD_WIDTH * i + 2 * j] = players[0]->game_state.board[i][j] + '0';
			player1_board[2 * BOARD_WIDTH * i + 2 * j + 1] = ',';
			player2_board[2 * BOARD_WIDTH * i + 2 * j] = players[1]->game_state.board[i][j] + '0';
			player2_board[2 * BOARD_WIDTH * i + 2 * j + 1] = ',';
		}
		player1_board[2 * BOARD_WIDTH * (i + 1) - 1] = ';';
		player2_board[2 * BOARD_WIDTH * (i + 1) - 1] = ';';
	}

	sprintf(game_state,
			"STATE MP %s_%d_%d_%d_%d,%d,%d_%d %s_%d_%d_%d_%d,%d,%d_%d",
			player1_board,
			players[0]->game_state.game_over,
			players[0]->game_state.score,
			players[0]->game_state.winning,
			players[0]->game_state.piece_queue[0],
			players[0]->game_state.piece_queue[1],
			players[0]->game_state.piece_queue[2],
			players[0]->game_state.hold_piece,
			player2_board,
			players[1]->game_state.game_over,
			players[1]->game_state.score,
			players[1]->game_state.winning,
			players[1]->game_state.piece_queue[0],
			players[1]->game_state.piece_queue[1],
			players[1]->game_state.piece_queue[2],
			players[1]->game_state.hold_piece);

	for(int i = 0; i < 2; i++)
	{
		sendto(sock,
			   game_state,
			   strlen(game_state),
			   0,
			   (struct sockaddr*)&players[i]->sock,
			   client_length[i]);
	}
}

void send_match_created(int sock, struct sockaddr_in client, int player)
{
	socklen_t client_length = sizeof(client);
	char match_created[9] = "MATCH P-\0";
	match_created[7] = player + '0';
	sendto(sock, match_created, strlen(match_created), 0, (struct sockaddr*)&client, client_length);
}

struct Client* init_client()
{
	struct Client* client = (struct Client*)malloc(sizeof(struct Client));

	struct sockaddr_in client_sock;
	client->sock = client_sock;
	client->has_connection = 0;
	client->ping_counter = 0;
	client->ping = 0;
	client->ping_send = 0;
	client->saving_score = 0;
	struct GameState game_state = {
		create_board(),
		0,
		0,
		NO_INPUT_CHAR,
		0,
		0,
		{0, 0, 0},
		0,
		0,
		false,
		true,
		1,
	};
	client->game_state = game_state;
	pthread_t thread_id;
	client->game_thread = thread_id;
	client->close_thread = 0;

	return client;
}

struct Server* init_server(int server_sock)
{
	struct Server* server = (struct Server*)malloc(sizeof(struct Server));

	server->sock = server_sock;
	server->n_clients = 0;
	server->queue = init_queue(MAX_CLIENTS);
	server->running = 1;
	server->closing_clients = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		server->clients[i] = init_client();
	}
	server->scores = load_scores();

	return server;
}

int find_client_index(struct Client* clients[MAX_CLIENTS], int n_clients,
					  struct sockaddr_in client_sock)
{
	for(int i = 0; i < n_clients; i++)
	{
		if(clients[i]->sock.sin_addr.s_addr == client_sock.sin_addr.s_addr &&
		   clients[i]->sock.sin_port == client_sock.sin_port)
		{
			return i;
		}
	}
	return -1;
}

void* ping_clients(void* args)
{
	struct Server* server = args;

	while(server->running)
	{
		int n_closed_connections = 0;
		int closed_connection_indexes[MAX_CLIENTS];

		for(int i = 0; i < server->n_clients; i++)
		{
			if(server->clients[i]->close_thread)
			{
				printf("[SERVER] joining finished game thread\n");
				pthread_join(server->clients[i]->game_thread, NULL);
				server->clients[i]->close_thread = 0;
			}
			char ping_message[1024] = {0};
			sprintf(ping_message, "PING %ld", server->clients[i]->ping);

			if(server->clients[i]->ping_counter > PING_TIMEOUT)
			{
				server->clients[i]->has_connection = 0;
				closed_connection_indexes[n_closed_connections] = i;
				n_closed_connections++;
				continue;
			}

			struct timespec ping_send;
			clock_gettime(CLOCK_MONOTONIC_RAW, &ping_send);

			sendto(server->sock,
				   ping_message,
				   strlen(ping_message),
				   0,
				   (struct sockaddr*)&(server->clients[i]->sock),
				   sizeof(server->clients[i]->sock));
			server->clients[i]->ping_send = ping_send.tv_nsec / 1000000;
			server->clients[i]->ping_counter++;
		}

		if(n_closed_connections > 0)
		{
			printf("[SERVER] Found %d closed connections. Cleaning up...\n", n_closed_connections);
			server->closing_clients = 1;
			for(int i = n_closed_connections - 1; i >= 0; i--)
			{
				log_connection(
					server->clients[closed_connection_indexes[i]]->sock, server->n_clients - 1, 0);

				remove_client(server->queue, server->clients[closed_connection_indexes[i]]);
				print_queue(server->queue);

				for(int j = closed_connection_indexes[i]; j < server->n_clients - 1; j++)
				{
					server->clients[j] = server->clients[j + 1];
				}
				server->n_clients--;
				server->clients[server->n_clients] = init_client();
			}
			server->closing_clients = 0;
		}

		sleep(1);
	}

	return 0;
}

void* listen_to_messages(void* args)
{
	struct Server* server = args;
	struct SingleplayerGame* singleplayer_game;
	struct MultiplayerGame* multiplayer_game;

	char connect_failure[12] = "CONNECT ERR\0";
	char connect_success[11] = "CONNECT OK\0";
	char start_failure[10] = "START ERR\0";
	char start_success[9] = "START OK\0";
	char highscore_success[13] = "HIGHSCORE OK\0";
	char highscore_failure[14] = "HIGHSCORE ERR\0";
	char unconnected[7] = "NOCONN\0";

	while(server->running)
	{
		char buf[1024] = {0};
		struct sockaddr_in client_sock;
		socklen_t client_length = sizeof(client_sock);

		ssize_t n_bytes_read =
			recvfrom(server->sock, buf, 1024, 0, (struct sockaddr*)&client_sock, &client_length);

		if(n_bytes_read == -1)
		{
			continue;
		}

		while(server->closing_clients)
		{
			usleep(1000);
		}

		char* identifier = strtok(buf, " ");

		if(strcmp(identifier, "CONNECT") == 0)
		{
			printf("[SERVER] Establishing new connection...\n");
			int n_clients =
				establish_new_connection(client_sock, server->clients, server->n_clients);
			if(n_clients == -1)
			{
				sendto(server->sock,
					   connect_failure,
					   strlen(connect_failure),
					   0,
					   (struct sockaddr*)&client_sock,
					   client_length);
				continue;
			}
			sendto(server->sock,
				   connect_success,
				   strlen(connect_success),
				   0,
				   (struct sockaddr*)&client_sock,
				   client_length);
			server->n_clients = n_clients;
			continue;
		}

		if(!is_connected(client_sock.sin_addr.s_addr,
						 client_sock.sin_port,
						 server->clients,
						 server->n_clients))
		{
			printf("[SERVER] client not connected\n");
			sendto(server->sock,
				   unconnected,
				   strlen(unconnected),
				   0,
				   (struct sockaddr*)&client_sock,
				   client_length);
			continue;
		}

		int client_index = find_client_index(server->clients, server->n_clients, client_sock);

		if(strcmp(identifier, "PING") == 0)
		{
			char* result = strtok(NULL, " ");
			struct timespec response_received;

			if(strcmp(result, "OK") == 0)
			{
				clock_gettime(CLOCK_MONOTONIC_RAW, &response_received);
				server->clients[client_index]->ping =
					response_received.tv_nsec / 1000000 - server->clients[client_index]->ping_send;
				server->clients[client_index]->ping_counter = 0;
			}
		}

		if(strcmp(identifier, "START") == 0)
		{
			if(server->clients[client_index]->game_state.playing)
			{
				sendto(server->sock,
					   start_failure,
					   strlen(start_failure),
					   0,
					   (struct sockaddr*)&(server->clients[client_index]->sock),
					   client_length);
				continue;
			}

			singleplayer_game = initialize_game(server->sock, server->clients[client_index]);

			int thread_created = pthread_create(&(server->clients[client_index]->game_thread),
												NULL,
												run_singleplayer_game,
												singleplayer_game);
			if(thread_created != 0)
			{
				printf("[SERVER] Failed to start thread.");
				sendto(server->sock,
					   start_failure,
					   strlen(start_failure),
					   0,
					   (struct sockaddr*)&client_sock,
					   client_length);
				continue;
			}

			sendto(server->sock,
				   start_success,
				   strlen(start_success),
				   0,
				   (struct sockaddr*)&client_sock,
				   client_length);
			server->clients[client_index]->game_state.playing = 1;
		}

		if(strcmp(identifier, "QUEUE") == 0)
		{
			char* action = strtok(NULL, " ");
			if(strcmp(action, "ENTER") == 0)
			{
				enqueue(server->queue, server->clients[client_index]);
				printf("[SERVER] Client entered queue\n");
			}
			else if(strcmp(action, "CANCEL") == 0)
			{
				remove_client(server->queue, server->clients[client_index]);
				printf("[SERVER] Client canceled queue\n");
			}

			print_queue(server->queue);
			if(server->queue->size >= 2)
			{
				struct Client* player1 = dequeue(server->queue);
				int player1_index =
					find_client_index(server->clients, server->n_clients, player1->sock);
				struct Client* player2 = dequeue(server->queue);
				int player2_index =
					find_client_index(server->clients, server->n_clients, player2->sock);

				multiplayer_game = initialize_multiplayer_game(
					server->sock, server->clients[player1_index], server->clients[player2_index]);

				int thread_created = pthread_create(&(server->clients[player1_index]->game_thread),
													NULL,
													run_multiplayer_game,
													multiplayer_game);

				if(thread_created != 0)
				{
					printf("[SERVER] Failed to start thread.");
					sendto(server->sock,
						   start_failure,
						   strlen(start_failure),
						   0,
						   (struct sockaddr*)&client_sock,
						   client_length);
					continue;
				}
				server->clients[player2_index]->game_thread =
					server->clients[player1_index]->game_thread;
			}
		}

		if(strcmp(identifier, "ACTION") == 0)
		{
			char* action = strtok(NULL, " ");
			server->clients[client_index]->game_state.current_action = action[0];
		}

		if(strcmp(identifier, "SCORES") == 0)
		{
			char* leaderboard = format_scores(server->scores);
			sendto(server->sock,
				   leaderboard,
				   strlen(leaderboard),
				   0,
				   (struct sockaddr*)&client_sock,
				   client_length);
		}

		if(strcmp(identifier, "HIGHSCORE") == 0)
		{
			char* name = strtok(NULL, "|");
			if(!server->clients[client_index]->saving_score || strlen(name) > 20)
			{
				sendto(server->sock,
					   highscore_failure,
					   strlen(highscore_failure),
					   0,
					   (struct sockaddr*)&client_sock,
					   client_length);
			}
			add_score(server->scores, name, server->clients[client_index]->game_state.score);
			sendto(server->sock,
				   highscore_success,
				   strlen(highscore_success),
				   0,
				   (struct sockaddr*)&client_sock,
				   client_length);
			server->clients[client_index]->saving_score = 0;
		}
	}

	return 0;
}
