#ifndef QUEUE_H
#define QUEUE_H

#include "server.h"
#include <stdbool.h>


struct Queue{
	struct Client** queue;
	int max_size;
	int size;
};

struct Queue* init_queue(int queue_size);

bool remove_client(struct Queue* queue, struct Client* client);

struct Client* dequeue(struct Queue* queue);

bool enqueue(struct Queue* queue, struct Client* client);

bool isEmpty(struct Queue* queue);

bool isFull(struct Queue* queue);

void print_queue(struct Queue* queue);

#endif
