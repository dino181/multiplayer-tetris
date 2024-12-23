
#ifndef MOVEMENTS_H
#define MOVEMENTS_H

#include "types.h"

int move_down(int** board, struct Piece* piece);

void move_left(int** board, struct Piece* piece);

void move_right(int** board, struct Piece* piece);

void rotate_cw(int** board, struct Piece* piece);

void rotate_ccw(int** board, struct Piece* piece);

void clear_piece(int** board, struct Piece* piece);

#endif
