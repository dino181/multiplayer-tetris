#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <stdbool.h>
#include "structs.h"

struct Connection{
	bool open;
	int client;
	struct sockaddr_in server;
	int ping;
	bool awaiting_connect_response;
	bool awaiting_start_response;
};

void send_action(struct Connection* connection, char action);

void send_highscore_name(struct Connection* connection, char* name);

void send_connect(struct Connection* connection);

void send_start(struct Connection* connection);

void send_queue(struct App* app, bool enter_queue);

void request_leaderboard(struct Connection* connection);

void* listen_for_board(void* args);

void* listen_on_connection(void* args);

void flush_connection(struct Connection* connection);

struct App* init_app();

void free_app(struct App* app);

void close_connection(struct Connection* connection, pthread_t thread_id);

struct Connection* get_connection();
#endif
