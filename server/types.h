#ifndef TYPES_H
#define TYPES_H

#define N_PIECES 7

struct Piece {
	int size;
	int value;
	int** squares;
	int center_position[2];
};

enum PieceType{
	I = 1,
	L = 2,
	J = 3,
	S = 4,
	Z = 5,
	O = 6,
	T = 7,
};

#endif
