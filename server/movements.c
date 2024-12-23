#include "movements.h"
#include "game.h"

void clear_piece(int** board, struct Piece* piece)
{
	for(int i = 0; i < piece->size; i++)
	{
		int x = piece->squares[i][0] + piece->center_position[0];
		int y = piece->squares[i][1] + piece->center_position[1];
		board[y][x] = 0;
	}
}

void add_piece(int** board, struct Piece* piece)
{
	for(int i = 0; i < piece->size; i++)
	{
		int x = piece->squares[i][0] + piece->center_position[0];
		int y = piece->squares[i][1] + piece->center_position[1];
		board[y][x] = piece->value;
	}
}

int can_move_down(int** board, struct Piece* piece)
{
	for(int i = 0; i < piece->size; i++)
	{
		int y = piece->squares[i][1] + piece->center_position[1] + 1;
		int x = piece->squares[i][0] + piece->center_position[0];
		if(y >= BOARD_HEIGHT)
		{
			return 0;
		}

		if(board[y][x] != 0)
		{
			return 0;
		}
	}
	return 1;
}

int move_down(int** board, struct Piece* piece)
{
	clear_piece(board, piece);

	if(!can_move_down(board, piece))
	{
		add_piece(board, piece);
		return 0;
	}

	piece->center_position[1]++;

	add_piece(board, piece);

	return 1;
}

int can_move_left(int** board, struct Piece* piece)
{
	for(int i = 0; i < piece->size; i++)
	{
		int y = piece->squares[i][1] + piece->center_position[1];
		int x = piece->squares[i][0] + piece->center_position[0] - 1;

		if(x < 0)
		{
			return 0;
		}
		if(board[y][x] != 0)
		{
			return 0;
		}
	}
	return 1;
}

void move_left(int** board, struct Piece* piece)
{
	clear_piece(board, piece);

	if(!can_move_left(board, piece))
	{
		add_piece(board, piece);
		return;
	}

	piece->center_position[0]--;

	add_piece(board, piece);
}

int can_move_right(int** board, struct Piece* piece)
{
	for(int i = 0; i < piece->size; i++)
	{
		int y = piece->squares[i][1] + piece->center_position[1];
		int x = piece->squares[i][0] + piece->center_position[0] + 1;

		if(x >= BOARD_WIDTH)
		{
			return 0;
		}
		if(board[y][x] != 0)
		{
			return 0;
		}
	}
	return 1;
}

void move_right(int** board, struct Piece* piece)
{
	clear_piece(board, piece);

	if(!can_move_right(board, piece))
	{
		add_piece(board, piece);
		return;
	}

	piece->center_position[0]++;

	add_piece(board, piece);
}

int is_placeable(int** board, struct Piece* piece)
{
	for(int i = 0; i < piece->size; i++)
	{
		int y = piece->squares[i][1] + piece->center_position[1];
		int x = piece->squares[i][0] + piece->center_position[0];

		if(x >= BOARD_WIDTH || x < 0 || y < 0 || y >= BOARD_HEIGHT)
		{
			return 0;
		}
		if(board[y][x] != 0)
		{
			return 0;
		}
	}
	return 1;
}

void rotate_piece_cw(struct Piece* piece)
{
	switch(piece->value)
	{
	case I:
		for(int i = 0; i < piece->size; i++)
		{
			int temp = piece->squares[i][1];
			piece->squares[i][1] = piece->squares[i][0];
			piece->squares[i][0] = temp;
		}
		break;
	case O:
		break;
	default:
		for(int i = 0; i < piece->size; i++)
		{
			if(piece->squares[i][0] == 0 && piece->squares[i][1] == 0)
			{
				continue;
			}
			if(piece->squares[i][0] == 0)
			{
				int temp = piece->squares[i][1];
				piece->squares[i][1] = piece->squares[i][0];
				piece->squares[i][0] = temp * -1;
			}

			else if(piece->squares[i][1] == 0)
			{
				int temp = piece->squares[i][1];
				piece->squares[i][1] = piece->squares[i][0];
				piece->squares[i][0] = temp;
			}

			else
			{
				if(piece->squares[i][1] * piece->squares[i][0] == 1)
				{
					piece->squares[i][0] *= -1;
				}
				else
				{
					piece->squares[i][1] *= -1;
				}
			}
		}
		break;
	}
}

void rotate_piece_ccw(struct Piece* piece)
{
	switch(piece->value)
	{
	case I:
		for(int i = 0; i < piece->size; i++)
		{
			int temp = piece->squares[i][1];
			piece->squares[i][1] = piece->squares[i][0];
			piece->squares[i][0] = temp;
		}
		break;
	case O:
		break;
	default:
		for(int i = 0; i < piece->size; i++)
		{
			if(piece->squares[i][0] == 0 && piece->squares[i][1] == 0)
			{
				continue;
			}
			if(piece->squares[i][0] == 0)
			{
				int temp = piece->squares[i][1];
				piece->squares[i][1] = piece->squares[i][0];
				piece->squares[i][0] = temp;
			}

			else if(piece->squares[i][1] == 0)
			{
				int temp = piece->squares[i][0];
				piece->squares[i][0] = piece->squares[i][1];
				piece->squares[i][1] = temp * -1;
			}

			else
			{
				if(piece->squares[i][1] * piece->squares[i][0] == 1)
				{
					piece->squares[i][1] *= -1;
				}
				else
				{
					piece->squares[i][0] *= -1;
				}
			}
		}
		break;
	}
}

void rotate_cw(int** board, struct Piece* piece)
{
	clear_piece(board, piece);

	rotate_piece_cw(piece);

	if(!is_placeable(board, piece))
	{
		rotate_piece_ccw(piece);
	}
	add_piece(board, piece);
}

void rotate_ccw(int** board, struct Piece* piece)
{
	clear_piece(board, piece);

	rotate_piece_ccw(piece);

	if(!is_placeable(board, piece))
	{
		rotate_piece_cw(piece);
	}
	add_piece(board, piece);
}
