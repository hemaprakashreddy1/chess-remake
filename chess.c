#include <stdio.h>
#include <stdlib.h>

void display_number_board(int *board);
void print_piece_name(int piece);
void display_name_board(int *board);

const int EMPTY = 0, KING = 1, QUEEN = 2, ROOK = 3, BISHOP = 4, KNIGHT = 5, PAWN = 7;
const int WHITE = 8, BLACK = 16;
const int N = 8, E = 1, W = -1, S = -8, NE = 9, NW = 7, SE = -7, SW = -9;

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
    int rank = 7;
    int file = 0;

    for(int i = 0; fen[i] != '\0'; i++)
    {
        if(fen[i] == ' ')
        {
            break;
        }
        else if(fen[i] == '/')
        {
            rank--;
            file = 0;
        }
        else if(fen[i] >= '1' && fen[i] <= '8')
        {
            int target = file + fen[i] - '0';
            while(file < target)
            {
                board[rank * 8 + file] = 0;
                file++;
            }
        }
        else
        {
            board[rank * 8 + file] = fen_char_to_number(fen[i]);
            file++;
        }
    }
}

char number_to_fen_char(int piece)
{
    int color = piece_color(piece);
    int type = piece_type(piece);

    if(type == PAWN)
    {
        return color == WHITE ? 'P' : 'p';
    }
    else if(type == KNIGHT)
    {
        return color == WHITE ? 'N' : 'n';
    }
    else if(type == BISHOP)
    {
        return color == WHITE ? 'B' : 'b';
    }
    else if(type == ROOK)
    {
        return color == WHITE ? 'R' : 'r';
    }
    else if(type == QUEEN)
    {
        return color == WHITE ? 'Q' : 'q';
    }
    else if(type == KING)
    {
        return color == WHITE ? 'K' : 'k';
    }
    else
    {
        printf("unknown type %d\n", type);
        return 0;
    }
}

char* generate_fen(int *board)
{
    char *fen = (char *)malloc(sizeof(char) * 100);
    int fen_index = 0;
    int empty_piece_count;

    for(int rank = 7; rank >= 0; rank--)
    {
        empty_piece_count = 0;
        for(int file = 0; file <= 7; file++)
        {
            int piece = board[rank * 8 + file];
            if( piece != EMPTY)
            {
                if(empty_piece_count != 0)
                {
                    fen[fen_index++] = empty_piece_count + '0';
                    empty_piece_count = 0;
                }
                fen[fen_index++] = number_to_fen_char(piece);
            }
            else
            {
                empty_piece_count++;
            }
        }
        if(empty_piece_count != 0)
        {
            fen[fen_index++] = empty_piece_count + '0';
        }
        if(rank != 0)
        {
            fen[fen_index++] = '/';
        }
    }

    fen[fen_index] = '\0';
    return fen;
}

int min(int a, int b)
{
    return a > b ? b : a;
}

void generate_steps_to_edges(int array[][8])
{
    int north, east, west, south, north_east, north_west, south_east, south_west;
    int index;

    for(int rank = 0; rank <= 7; rank++)
    {
        for(int file = 0; file <= 7; file++)
        {
            north = 7 - rank;
            east = 7 - file;
            west = file;
            south = rank;
            north_east = min(north, east);
            north_west = min(north, west);
            south_east = min(south, west);
            south_west = min(south, west);
            index = rank * 8 + file;
            array[index][0] = north;
            array[index][1] = east;
            array[index][2] = west;
            array[index][3] = south;
            array[index][4] = north_east;
            array[index][5] = north_west;
            array[index][6] = south_east;
            array[index][7] = south_west;
        }
    }
}

int main()
{
    int board[64];
    int distance_to_borders[64][8];
    generate_steps_to_edges(distance_to_borders);
    init_fen_board(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    display_name_board(board);
    char *fen = generate_fen(board);
    printf("%s\n", fen);
    free(fen);
    printf("%d", 0 + N);
    return 0;
}

void display_number_board(int *board)
{
    for(int rank = 7; rank >= 0; rank--)
    {
        for(int file = 0; file <= 7; file++)
        {
            printf("%d ", board[rank * 8 + file]);
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
    for(int rank = 7; rank >= 0; rank--)
    {
        printf("%d\t", rank);
        for(int file = 0; file < 8; file++)
        {
            print_piece_name(board[rank * 8 + file]);
        }
        printf("\n\n");
    }
}