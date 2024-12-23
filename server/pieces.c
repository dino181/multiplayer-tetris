#include "pieces.h"
#include "types.h"
#include <stdlib.h>

void add_piece_squares(struct Piece* piece)
{
	switch(piece->value)
	{
	case I:
		piece->squares[0][0] = -2;
		piece->squares[0][1] = 0;
		piece->squares[1][0] = -1;
		piece->squares[1][1] = 0;
		piece->squares[2][0] = 0;
		piece->squares[2][1] = 0;
		piece->squares[3][0] = 1;
		piece->squares[3][1] = 0;
		piece->center_position[0] = 5;
		piece->center_position[1] = 0;
		break;
	case L:
		piece->squares[0][0] = -1;
		piece->squares[0][1] = 0;
		piece->squares[1][0] = -1;
		piece->squares[1][1] = 1;
		piece->squares[2][0] = 0;
		piece->squares[2][1] = 0;
		piece->squares[3][0] = 1;
		piece->squares[3][1] = 0;
		piece->center_position[0] = 5;
		piece->center_position[1] = 0;
		break;
	case J:
		piece->squares[0][0] = -1;
		piece->squares[0][1] = 0;
		piece->squares[1][0] = 0;
		piece->squares[1][1] = 0;
		piece->squares[2][0] = 1;
		piece->squares[2][1] = 0;
		piece->squares[3][0] = 1;
		piece->squares[3][1] = 1;
		piece->center_position[0] = 4;
		piece->center_position[1] = 0;
		break;
	case S:
		piece->squares[0][0] = -1;
		piece->squares[0][1] = 1;
		piece->squares[1][0] = 0;
		piece->squares[1][1] = 1;
		piece->squares[2][0] = 0;
		piece->squares[2][1] = 0;
		piece->squares[3][0] = 1;
		piece->squares[3][1] = 0;
		piece->center_position[0] = 4;
		piece->center_position[1] = 0;
		break;
	case Z:
		piece->squares[0][0] = -1;
		piece->squares[0][1] = 0;
		piece->squares[1][0] = 0;
		piece->squares[1][1] = 0;
		piece->squares[2][0] = 0;
		piece->squares[2][1] = 1;
		piece->squares[3][0] = 1;
		piece->squares[3][1] = 1;
		piece->center_position[0] = 4;
		piece->center_position[1] = 0;
		break;
	case O:
		piece->squares[0][0] = 0;
		piece->squares[0][1] = 0;
		piece->squares[1][0] = 1;
		piece->squares[1][1] = 0;
		piece->squares[2][0] = 0;
		piece->squares[2][1] = 1;
		piece->squares[3][0] = 1;
		piece->squares[3][1] = 1;
		piece->center_position[0] = 4;
		piece->center_position[1] = 0;
		break;
	case T:
		piece->squares[0][0] = -1;
		piece->squares[0][1] = 0;
		piece->squares[1][0] = 0;
		piece->squares[1][1] = 0;
		piece->squares[2][0] = 1;
		piece->squares[2][1] = 0;
		piece->squares[3][0] = 0;
		piece->squares[3][1] = 1;
		piece->center_position[0] = 4;
		piece->center_position[1] = 0;
		break;
	}
}
void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void update_piece_queue(int piece_queue[2 * N_PIECES], int replace_upper_half,
						unsigned long int seed)
{
	srand(seed);
	int offset = replace_upper_half * N_PIECES;
	for(int i = N_PIECES - 1; i > 0; i--)
	{
		int j = rand() % (i + 1);
		swap(&piece_queue[offset + i], &piece_queue[offset + j]);
	}
}

struct Piece* generate_piece(int current_piece)
{
	struct Piece* piece = (struct Piece*)malloc(sizeof(struct Piece));
	int piece_type = current_piece;
	int piece_size = 4;
	int piece_value = piece_type;

	piece->size = piece_size;
	piece->value = piece_value;
	piece->squares = (int**)malloc(piece_size * sizeof(int*));
	for(int i = 0; i < piece_size; i++)
	{
		piece->squares[i] = (int*)malloc(2 * sizeof(int));
	}

	add_piece_squares(piece);

	return piece;
}
