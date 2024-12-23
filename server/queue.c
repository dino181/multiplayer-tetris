#include "queue.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct Queue *init_queue(int queue_size) {
  struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));

  queue->max_size = queue_size;
  queue->size = 0;
  queue->queue = (struct Client **)malloc(queue_size * sizeof(struct Client *));
  for (int i = 0; i < queue_size; i++) {
    queue->queue[i] = (struct Client *)malloc(sizeof(struct Client));
  }

  return queue;
}

struct Client *dequeue(struct Queue *queue) {
  if (isEmpty(queue)) {
    return NULL;
  }

  struct Client *client = queue->queue[0];

  for (int i = 0; i < queue->size - 1; i++) {
    queue->queue[i] = queue->queue[i + 1];
  }

  queue->queue[queue->size] = (struct Client *)malloc(sizeof(struct Client));
  queue->size--;

  return client;
}

bool enqueue(struct Queue *queue, struct Client *client) {
  if (isFull(queue)) {
    return false;
  }

  queue->queue[queue->size] = client;
  queue->size++;

  return true;
}

bool remove_client(struct Queue *queue, struct Client *client) {
  bool found_client = false;
  for (int i = 0; i < queue->size; i++) {
    if (queue->queue[i]->sock.sin_addr.s_addr == client->sock.sin_addr.s_addr &&
        queue->queue[i]->sock.sin_port == client->sock.sin_port) {
      found_client = true;
    }

    if (found_client && i + 1 < queue->size) {
      queue->queue[i] = queue->queue[i + 1];
    }
  }

  if (found_client) {
    queue->queue[queue->size] = (struct Client *)malloc(sizeof(struct Client));
    queue->size--;
  }

  return found_client;
}

bool isEmpty(struct Queue *queue) { return queue->size == 0; }

bool isFull(struct Queue *queue) { return queue->size == queue->max_size; }

void print_queue(struct Queue *queue) {
  for (int i = 0; i < queue->size; i++) {
    printf("[SERVER] Queue %d: %d:%d\n", i,
           queue->queue[i]->sock.sin_addr.s_addr,
           queue->queue[i]->sock.sin_port);
  }
}
