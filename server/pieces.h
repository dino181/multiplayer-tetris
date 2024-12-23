#include "types.h"
#ifndef PIECES_H
#define PIECES_H 

struct Piece* generate_piece(int current_piece);

void update_piece_queue(int piece_queue[2*N_PIECES], int replace_upper_half, unsigned long int seed);
#endif
