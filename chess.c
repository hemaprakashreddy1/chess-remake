#include <stdio.h>

void display_number_board(int *board);
void print_piece_name(int piece);
void display_name_board(int *board);

const int KING = 1, QUEEN = 2, ROOK = 3, BISHOP = 4, KNIGHT = 5, PAWN = 7;
const int WHITE = 8, BLACK = 16;

int piece_color(int piece)
{
    return piece & 24;
}

int piece_type(int piece)
{
    return piece & 7;
}

void init_board(int *board)
{
    board[0] = board[7] = BLACK | ROOK , board[1] = board[6] = BLACK | KNIGHT,
    board[2] = board[5] = BLACK | BISHOP, board[3] = BLACK | QUEEN, board[4] = BLACK | KING;
    for(int i = 8; i <= 15; i++)
    {
        board[i] = BLACK | PAWN;
    }
    for(int i = 16; i <= 47; i++)
    {
        board[i] = 0;
    }
    for(int i = 48; i <= 55; i++)
    {
        board[i] = WHITE | PAWN;
    }
    board[56] = board[63] = WHITE | ROOK , board[57] = board[62] = WHITE | KNIGHT,
    board[58] = board[61] = WHITE | BISHOP, board[59] = WHITE | QUEEN, board[60] = WHITE | KING;
}

int fen_char_to_number(char ch)
{
    if(ch >= 'A' && ch <= 'Z')
    {
        if(ch == 'P')
        {
            return WHITE | PAWN;
        }
        else if(ch == 'N')
        {
            return WHITE | KNIGHT;
        }
        else if(ch == 'B')
        {
            return WHITE | BISHOP;
        }
        else if(ch == 'R')
        {
            return WHITE | ROOK;
        }
        else if(ch == 'Q')
        {
            return WHITE | QUEEN;
        }
        else if(ch == 'K')
        {
            return WHITE | KING;
        }
        return 0;
    }
    else
    {
        if(ch == 'p')
        {
            return BLACK | PAWN;
        }
        else if(ch == 'n')
        {
            return BLACK | KNIGHT;
        }
        else if(ch == 'b')
        {
            return BLACK | BISHOP;
        }
        else if(ch == 'r')
        {
            return BLACK | ROOK;
        }
        else if(ch == 'q')
        {
            return BLACK | QUEEN;
        }
        else if(ch == 'k')
        {
            return BLACK | KING;
        }
        return 0;
    }
}

void init_fen_board(int *board, char *fen)
{
    int row = 0;
    int col = 0;

    for(int i = 0; fen[i] != '\0'; i++)
    {
        if(fen[i] == ' ')
        {
            break;
        }
        else if(fen[i] == '/')
        {
            row++;
            col = 0;
        }
        else if(fen[i] >= '1' && fen[i] <= '8')
        {
            int target = col + fen[i] - '0';
            while(col < target)
            {
                board[row * 8 + col] = 0;
                col++;
            }
        }
        else
        {
            board[row * 8 + col] = fen_char_to_number(fen[i]);
            col++;
        }
    }
}

int main()
{
    int board[64];
    init_fen_board(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    display_name_board(board);
    return 0;
}

void display_number_board(int *board)
{
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            printf("%d ", board[i * 8 + j]);
        }
        printf("\n");
    }
}

void print_piece_name(int piece)
{
    if(piece == 0)
    {
        printf("\t");
        return;
    }

    int color = piece_color(piece);
    int type = piece_type(piece);
    if(color == BLACK)
    {
        printf("B");
    }
    else
    {
        printf("W");
    }
    if(type == PAWN)
    {
        printf("pawn\t");
    }
    else if(type == KNIGHT)
    {
        printf("knight\t");
    }
    else if(type == BISHOP)
    {
        printf("bishop\t");
    }
    else if(type == ROOK)
    {
        printf("rook\t");
    }
    else if(type == QUEEN)
    {
        printf("queen\t");
    }
    else if(type == KING)
    {
        printf("king\t");
    }
}

void display_name_board(int *board)
{
    printf("\t");
    for(int i = 0; i < 8; i++)
    {
        printf("%d\t", i);
    }
    printf("\n\n\n");
    for(int i = 0; i < 8; i++)
    {
        printf("%d\t", i);
        for(int j = 0; j < 8; j++)
        {
            print_piece_name(board[i * 8 + j]);
        }
        printf("\n\n");
    }
}