#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

static volatile int interrupted = 0;

void terminate_server(int signum)
{
	printf("\n[SERVER] Server was interrupted (signum: %d). Shutting down...\n", signum);
	interrupted = 1;
}

int main()
{
	signal(SIGTERM, terminate_server);
	signal(SIGINT, terminate_server);

	pthread_t ping_thread;
	pthread_t listen_thread;

	int sock = open_socket();
	struct Server* server = init_server(sock);

	int thread_created = pthread_create(&ping_thread, NULL, ping_clients, server);
	if(thread_created != 0)
	{
		printf("[SERVER] Failed to start pinging thread.\n");
		exit(EXIT_FAILURE);
	}

	int listen_thread_created = pthread_create(&listen_thread, NULL, listen_to_messages, server);
	if(listen_thread_created != 0)
	{
		printf("[SERVER] Failed to start listening thread.\n");
		exit(EXIT_FAILURE);
	}

	while(!interrupted)
	{
		sleep(1);
	}

	server->running = 0;

	pthread_join(listen_thread, NULL);
	pthread_join(ping_thread, NULL);

	close(sock);

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		free(server->clients[i]);
	}
	free(server);

	printf("[SERVER] Gracefully closed the server.\n");
	exit(EXIT_SUCCESS);
}
