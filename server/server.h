#ifndef SERVER_H
#define SERVER_H

#include "game.h"
#include <netinet/in.h>

#define MAX_CLIENTS 10

struct Client {
	struct sockaddr_in sock;
	int has_connection;
	uint64_t ping;
	int ping_counter;
	uint64_t ping_send;
	pthread_t game_thread;
	struct GameState game_state;
	int saving_score;
	int close_thread;

};

struct Server {
	int sock;
	struct Client* clients[MAX_CLIENTS];
	struct Queue* queue;
	int n_clients;
	int closing_clients;
	int running;
	struct HighScore** scores;

};

int open_socket();

int is_connected(int address, int port, struct Client* clients[MAX_CLIENTS], int n_clients);

int establish_new_connection(struct sockaddr_in client, struct Client* clients[MAX_CLIENTS], int n_clients);

void send_singleplayer_game_state(int sock, struct Client* client);

void send_multiplayer_game_state(int sock, struct Client* players[2]);

void send_match_created(int sock, struct sockaddr_in client, int player);

struct Client* init_client();

struct Server* init_server(int server_sock);

void* ping_clients(void* args);

void* listen_to_messages(void *args);

#endif
