#include <stdio.h>
#include <stdlib.h>

struct piece_list
{
    int max_pieces[7];
    int no_of_pieces[7];
    int list[7][10];
};

struct captured_pieces
{
    int pieces[32];
    int top;
};

struct fen
{
    char piece_placement[100];
    int turn;
    int white_castle;
    int black_castle;
    int en_passant;
    int half_moves;
    int full_moves;
};

struct chess_game
{
    int board[64];
    int turn;
    char fen[100];
    int white_castle;
    int black_castle;
    struct piece_list white_piece_list;
    struct piece_list black_piece_list;
    int distance_to_borders[64][8];
    struct move *last_move;
    struct captured_pieces captured_piece_list;
};

struct move
{
    int src;
    int dest;
    int type;
};

struct node
{
    struct move *mv;
    struct node *next;
};

struct queue
{
    struct node *front;
    struct node *rear;
};

void display_number_board(int *board);
void print_piece_name(int piece);
void display_name_board(int *board);
void generate_sliding_moves(struct chess_game *game, int positon, struct queue *q);
void generate_knight_moves(struct chess_game *game, int positon, struct queue *q);
void generate_king_moves(struct chess_game *game, int position, struct queue *q);
void generate_pawn_moves(struct chess_game *game, int position, struct queue *q);

const int EMPTY = 0, KING = 1, QUEEN = 2, ROOK = 3, BISHOP = 4, KNIGHT = 5, PAWN = 6;
const int WHITE = 8, BLACK = 16;
const int N = 8, E = 1, W = -1, S = -8, NE = 9, NW = 7, SE = -7, SW = -9;
const int DIRECTIONS[] = {N, E, W, S, NE, NW, SE, SW};
const int QUIET_MOVE = 0, DOUBLE_PAWN_PUSH = 1, KING_CASTLE = 2, QUEEN_CASTLE = 3, CAPTURES = 4,
ENPASSANT_CAPTURE = 5, KNIGHT_PROMOTION = 8, BISHOP_PROMOTION = 9, ROOK_PROMOTION = 10, QUEEN_PROMOTION = 11,
KNIGHT_PROMO_CAPTURE = 12, BISHOP_PROMO_CAPTURE = 13, ROOK_PROMO_CAPTURE = 14, QUEEN_PROMO_CAPTURE = 15;
const char PIECE_SYMBOLS[] = {'\0', 'k', 'q', 'r', 'b', 'n', 'p'};

int piece_color(int piece)
{
    return piece & 24;
}

int piece_type(int piece)
{
    return piece & 7;
}

int is_valid_position(int position)
{
    return position >= 0 && position <= 63;
}

int file(int position)
{
    return position % 8;
}

int rank(int position)
{
    return position / 8;
}

struct move* init_move(int source, int dest, int type)
{
    struct move *new = (struct move*)malloc(sizeof(struct move));
    if(new == NULL)
    {
        printf("memory not allocated\n");
        return NULL;
    }
    new->src = source;
    new->dest = dest;
    new->type = type;
    return new;
}

struct node* init_node(struct move *mv)
{
    struct node *new = (struct node*)malloc(sizeof(struct node));
    if(mv == NULL)
    {
        return NULL;
    }
    if(new == NULL)
    {
        printf("memory not allocated\n");
        return NULL;
    }

    new->mv = mv;
    new->next = NULL;
    return new;
}

struct queue* init_queue()
{
    struct queue *new = (struct queue*)malloc(sizeof(struct queue));
    if(new == NULL)
    {
        printf("memory not allocated\n");
        return NULL;
    }
    new->front = new->rear = NULL;
    return new;
}

void enqueue(struct queue *q, struct node *new)
{
    if(new == NULL)
    {
        return;
    }
    if(q->front == NULL)
    {
        q->front = q->rear = new;
    }
    else
    {
        q->rear->next = new;
        q->rear = new;
    }
}

struct move* dequeue(struct queue *q)
{
    struct node *temp;
    if(q->front == NULL)
    {
        return NULL;
    }
    else if(q->front == q->rear)
    {
        temp = q->front;
        q->front = q->rear = NULL;
    }
    else
    {
        temp = q->front;
        q->front = q->front->next;
    }

    struct move *mv = temp->mv;
    free(temp);
    return mv;
}

void display_queue(struct queue *q)
{
    struct node *front = q->front;
    int count = 0;
    while(front != NULL)
    {
        printf("src %d dest %d\n", front->mv->src, front->mv->dest);
        front = front->next;
        count++;
    }
    printf("queue count %d\n", count);
}

void free_queue(struct queue *q)
{
    struct node *front= q->front;
    struct node *temp;
    while(front != NULL)
    {
        free(front->mv);
        temp = front;
        front = front->next;
        free(temp);
    }
    free(q);
}

void add_captured_piece(struct captured_pieces *cp, int piece)
{
    if(cp->top == 31)
    {
        printf("stack overflow");
    }
    else
    {
        cp->pieces[++(cp->top)] = piece;
    }
}

int last_captured_piece(struct captured_pieces *cp)
{
    int piece = 0;
    if(cp->top == -1)
    {
        printf("stack underflow");
    }
    else
    {
        piece = cp->pieces[(cp->top)--];
    }
    return piece;
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
    int color;
    if(ch >= 'A' && ch <= 'Z')
    {
        color = WHITE;
        ch += 32;
    }
    else
    {
        color = BLACK;
    }
    if(ch == 'p')
    {
        return color | PAWN;
    }
    else if(ch == 'n')
    {
        return color | KNIGHT;
    }
    else if(ch == 'b')
    {
        return color | BISHOP;
    }
    else if(ch == 'r')
    {
        return color | ROOK;
    }
    else if(ch == 'q')
    {
        return color | QUEEN;
    }
    else if(ch == 'k')
    {
        return color | KING;
    }
    return 0;
}

void init_board_from_fen(int *board, char *fen)
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

    if(color == BLACK)
    {
        return PIECE_SYMBOLS[type];
    }

    return PIECE_SYMBOLS[type] - 32;
}

void generate_fen(struct chess_game *game)
{
    int *board = game->board;
    char *fen = game->fen;

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

    fen[fen_index++] = ' ';
    
    if(game->turn == BLACK)
    {
        fen[fen_index++] = 'b';
    }
    else
    {
        fen[fen_index++] = 'w';
    }

    fen[fen_index++] = ' ';

    int white_castle = game->white_castle;
    int black_castle = game->black_castle;
    if(white_castle == 0 && black_castle == 0)
    {
        fen[fen_index++] = '-';
    }
    else
    {
        if(white_castle == 3)
        {
            fen[fen_index++] = 'K';
            fen[fen_index++] = 'Q';
        }
        else if(white_castle == 2)
        {
            fen[fen_index++] = 'K';
        }
        else if(white_castle == 1)
        {
            fen[fen_index++] = 'Q';
        }

        if(black_castle == 3)
        {
            fen[fen_index++] = 'k';
            fen[fen_index++] = 'q';
        }
        else if(black_castle == 2)
        {
            fen[fen_index++] = 'k';
        }
        else if(black_castle == 1)
        {
            fen[fen_index++] = 'q';
        }
    }

    fen[fen_index] = '\0';
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
            south_east = min(south, east);
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

void init_piece_list(struct chess_game *game)
{
    int *board = game->board;
    struct piece_list *white_piece_list = &game->white_piece_list;
    struct piece_list *black_piece_list = &game->black_piece_list;

    white_piece_list->max_pieces[KING] = black_piece_list->max_pieces[KING] = 1;
    white_piece_list->max_pieces[QUEEN] = black_piece_list->max_pieces[QUEEN] = 9;
    white_piece_list->max_pieces[PAWN] = black_piece_list->max_pieces[PAWN] = 8;

    for(int i = 3; i < 6; i++)
    {
        white_piece_list->max_pieces[i] = black_piece_list->max_pieces[i] = 10;
    }

    for(int i = 1; i < 7; i++)
    {
        white_piece_list->no_of_pieces[i] = black_piece_list->no_of_pieces[i] = 0;
    }

    int color, type, piece;
    for(int rank = 0; rank <= 7; rank++)
    {
        for(int file = 0; file <= 7; file++)
        {
            piece = board[rank * 8 + file];
            if(piece != EMPTY)
            {
                color = piece_color(piece);
                type = piece_type(piece);
                if(color == WHITE)
                {
                    white_piece_list->list[type][white_piece_list->no_of_pieces[type]++] = rank * 8 + file;
                }
                else
                {
                    black_piece_list->list[type][black_piece_list->no_of_pieces[type]++] = rank * 8 + file;
                }
            }
        }
    }
}

void update_piece_index(struct piece_list *p_list, int piece_type, struct move* mv)
{
    int no_of_pieces = p_list->no_of_pieces[piece_type];
    int *pieces = p_list->list[piece_type];
    int src = mv->src;

    int updated = 0;
    for(int i = 0; i < no_of_pieces; i++)
    {
        if(pieces[i] == src)
        {
            pieces[i] = mv->dest;
            updated = 1;
            break;
        }
    }
    if(!updated)
    {
        printf("piece list not updated for mv src - %d, dest - %d\n", mv->src, mv->dest);
    }
}

void add_piece_index(struct piece_list *p_list, int piece_type, int index)
{
    p_list->list[piece_type][p_list->no_of_pieces[piece_type]++] = index;
}

void remove_piece_index(struct piece_list *p_list, int piece_type, int index)
{
    int no_of_pieces = p_list->no_of_pieces[piece_type];
    int *pieces = p_list->list[piece_type];
    int last_piece_index = pieces[no_of_pieces - 1];
    int removed = 0;

    for(int i = 0; i < no_of_pieces; i++)
    {
        if(pieces[i] == index)
        {
            pieces[i] = last_piece_index;
            p_list->no_of_pieces[piece_type]--;
            removed = 1;
            break;
        }
    }

    if(!removed)
    {
        printf("piece index %d not removed from piece list\n", index);
    }
}

void display_piece_list(struct piece_list *p_list)
{
    for(int type = 1; type < 7; type++)
    {
        int *arr = p_list->list[type];
        int no_of_pieces = p_list->no_of_pieces[type];
        printf("type %d - %c, piece count %d, ", type, PIECE_SYMBOLS[type], no_of_pieces);
        printf("indexes {");
        for(int j = 0; j < no_of_pieces; j++)
        {
            printf("%d, ", arr[j]);
        }
        printf("}");
        printf("\n");
    }
}

int is_sliding_piece(int type)
{
    return type == BISHOP || type == ROOK || type == QUEEN;
}

struct queue* generate_moves(struct chess_game *game)
{
    struct queue* q = init_queue();
    if(q == NULL)
    {
        return NULL;
    }
    
    struct piece_list *p_list;
    if(game->turn == WHITE)
    {
        p_list = &game->white_piece_list;
    }
    else
    {
        p_list = &game->black_piece_list;
    }

    int no_of_pieces;
    int *pieces_index;
    for(int type = 1; type < 7; type++)
    {
        no_of_pieces = p_list->no_of_pieces[type];
        if(no_of_pieces == 0)
        {
            continue;
        }
        pieces_index = p_list->list[type];
        
        if(is_sliding_piece(type))
        {
            for(int j = 0; j < no_of_pieces; j++)
            {
                generate_sliding_moves(game, pieces_index[j], q);
            }
        }
        else if(type == KNIGHT)
        {
            for(int j = 0; j < no_of_pieces; j++)
            {
                generate_knight_moves(game, pieces_index[j], q);
            }
        }
        else if(type == KING)
        {
            generate_king_moves(game, pieces_index[0], q);
        }
        else if(type == PAWN)
        {
            for(int j = 0; j < no_of_pieces; j++)
            {
                generate_pawn_moves(game, pieces_index[j], q);
            }
        }
    }
    return q;
}

void generate_sliding_moves(struct chess_game *game, int position, struct queue *q)
{
    int *board = game->board;
    int type = piece_type(board[position]);
    int start_direction, end_direction;

    if(type == ROOK)
    {
        start_direction = 0;
        end_direction = 3;
    }
    else if(type == BISHOP)
    {
        start_direction = 4;
        end_direction = 7;
    }
    else if(type == QUEEN)
    {
        start_direction = 0;
        end_direction = 7;
    }

    int no_of_steps, direction, current_position;
    for(int i = start_direction; i <= end_direction; i++)
    {
        no_of_steps = game->distance_to_borders[position][i];
        direction = DIRECTIONS[i];
        current_position = position;
        for(int j = 0; j < no_of_steps; j++)
        {
            current_position += direction;
            if(board[current_position] != EMPTY)
            {
                if(piece_color(board[current_position]) != game->turn)
                {
                    enqueue(q, init_node(init_move(position, current_position, CAPTURES)));
                }
                break;
            }
            enqueue(q, init_node(init_move(position, current_position, QUIET_MOVE)));
        }
    }
}

void generate_knight_moves(struct chess_game *game, int pos, struct queue *q)//
{
    int *board = game->board;
    int r = rank(pos);
    int f = file(pos);
    int turn_color = game->turn;
    int dest;
    int move_type;

    dest = pos + S + S + E;
    if(r > 1 && f < 7 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
    dest = pos + N + N + E;
    if(r < 6 && f < 7 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
    dest = pos + N + N + W;
    if(r < 6 && f > 0 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
    dest = pos + S + S + W;
    if(r > 1 && f > 0 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
    dest = pos + S + E + E;
    if(r > 0 && f < 6 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
    dest = pos + N + E + E;
    if(r < 7 && f < 6 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
    dest = pos + N + W + W;
    if(r < 7 && f > 1 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
    dest = pos + S + W + W;
    if(r > 0 && f > 1 && piece_color(board[dest]) != turn_color)
    {
        move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
        enqueue(q, init_node(init_move(pos, dest, move_type)));
    }
}

void generate_king_moves(struct chess_game *game, int position, struct queue *q)
{
    int *board = game->board;
    int move_type;
    int turn = game->turn;

    for(int i = 0; i <= 7; i++)
    {
        int dest = position + DIRECTIONS[i];
        if(game->distance_to_borders[position][i] > 0 && piece_color(board[dest]) != turn)
        {
            move_type = board[dest] == EMPTY ? QUIET_MOVE : CAPTURES;
            enqueue(q, init_node(init_move(position, dest, move_type)));
        }
    }

    int castle;
    if(turn == WHITE)
    {
        castle = game->white_castle;
    }
    else
    {
        castle = game->black_castle;
    }

    if(castle == 0)
    {
        return;
    }
    if((castle & 2) == 2 && board[position + E] == EMPTY && board[position + E + E] == EMPTY)
    {
        enqueue(q, init_node(init_move(position, position + E + E, KING_CASTLE)));
    }
    if((castle & 1) == 1 && board[position + W] == EMPTY && board[position + W + W] == EMPTY)
    {
        enqueue(q, init_node(init_move(position, position + W + W, QUEEN_CASTLE)));
    }
}

void add_pawn_promotions(int position, int dest, int captures, struct queue *q)
{
    enqueue(q, init_node(init_move(position, dest, QUEEN_PROMOTION | captures)));
    enqueue(q, init_node(init_move(position, dest, ROOK_PROMOTION | captures)));
    enqueue(q, init_node(init_move(position, dest, KNIGHT_PROMOTION | captures)));
    enqueue(q, init_node(init_move(position, dest, BISHOP_PROMOTION | captures)));
}

void generate_promotion_moves(struct chess_game *game, int position, int direction, struct queue *q)
{
    int turn = game->turn;
    int *board = game->board;
    int fle = file(position);
    if(board[position + direction]  == EMPTY)
    {
        add_pawn_promotions(position, position + direction, 0, q);
    }
    if(fle < 7 && board[position + direction + E] != EMPTY && piece_color(board[position + direction + E]) != turn)
    {
        add_pawn_promotions(position, position + direction + E, CAPTURES, q);
    }
    if(fle > 0 && board[position + direction + W] != EMPTY && piece_color(board[position + direction + W]) != turn)
    {
        add_pawn_promotions(position, position + direction + W, CAPTURES, q);
    }
}

void generate_pawn_moves(struct chess_game *game, int position, struct queue *q)
{
    int color = game->turn;
    int rnk = rank(position);
    int fle = file(position);
    int *board = game->board;

    if(color == WHITE)
    {
        int north = position + N;
        int north_east = position + NE;
        int north_west = position + NW;

        if(rnk < 6 && board[north] == EMPTY)
        {
            enqueue(q, init_node(init_move(position, north, QUIET_MOVE)));
        }
        if(rnk < 6 && fle < 7 && piece_color(board[north_east]) == BLACK)
        {
            enqueue(q, init_node(init_move(position, north_east, CAPTURES)));
        }
        if(rnk < 6 && fle > 0 && piece_color(board[north_west]) == BLACK)
        {
            enqueue(q, init_node(init_move(position, north_west, CAPTURES)));
        }

        if(rnk == 1 && board[north] == EMPTY && board[north + N] == EMPTY)
        {
            enqueue(q, init_node(init_move(position, north + N, DOUBLE_PAWN_PUSH)));
        }
        else if(rnk == 6)
        {
            generate_promotion_moves(game, position, N, q);
        }
        else if(rnk == 4)
        {
            struct move* last_move = game->last_move;
            if(last_move == NULL)
            {
                return;
            }
            int last_move_type = last_move->type;
            if(last_move_type == DOUBLE_PAWN_PUSH)
            {
                int last_dest = last_move->dest;
                int last_fle = file(last_dest);
                if(fle < 7 && last_fle > 0 && last_dest + W == position)
                {
                    enqueue(q, init_node(init_move(position, position + NE, ENPASSANT_CAPTURE)));
                }
                if(fle > 0 && last_fle < 7 && last_dest + E == position)
                {
                    enqueue(q, init_node(init_move(position, position + NW, ENPASSANT_CAPTURE)));
                }
            }
        }
    }
    else
    {
        int south = position + S;
        int south_east = position + SE;
        int south_west = position + SW;

        if(rnk > 1 && board[south] == EMPTY)
        {
            enqueue(q, init_node(init_move(position, south, QUIET_MOVE)));
        }
        if(rnk > 1 && fle > 7 && piece_color(board[south_east]) == WHITE)
        {
            enqueue(q, init_node(init_move(position, south_east, CAPTURES)));
        }
        if(rnk > 1 && fle > 0 && piece_color(board[south_west]) == WHITE)
        {
            enqueue(q, init_node(init_move(position, south_west, CAPTURES)));
        }

        if(rnk == 6 && board[south] == EMPTY && board[south + S] == EMPTY)
        {
            enqueue(q, init_node(init_move(position, south + S, DOUBLE_PAWN_PUSH)));
        }
        else if(rnk == 1)
        {
            generate_promotion_moves(game, position, S, q);
        }
        else if(rnk == 3)
        {
            struct move* last_move = game->last_move;
            if(last_move == NULL)
            {
                return;
            }
            int last_move_type = last_move->type;
            if(last_move_type == DOUBLE_PAWN_PUSH)
            {
                int last_dest = last_move->dest;
                int last_fle = file(last_dest);
                if(fle < 7 && last_fle > 0 && last_dest + W == position)
                {
                    enqueue(q, init_node(init_move(position, position + SE, ENPASSANT_CAPTURE)));
                }
                if(fle > 0 && last_fle < 7 && last_dest + E == position)
                {
                    enqueue(q, init_node(init_move(position, position + SW, ENPASSANT_CAPTURE)));
                }
            }
        }
    }
}

int en_passant_position(char *pgn)
{
    int rnk = pgn[1] - '0' - 1;
    int fle = pgn[0] - 'a';
    return rnk * 8 + fle;
}

void free_strings(char **strings, int len)
{
    for(int i = 0; i < len; i++)
    {
        free(strings[i]);
    }
}

void copy_string_range(char *dest, char *source, int start, int end)
{
    int index = 0;
    for(int i = start; i <= end; i++)
    {
        dest[index++] = source[i];
    }
    dest[index]= '\0';
}

void string_cpy(char *dest, char *source)
{
    int i;
    for(i = 0; source[i] != '\0'; i++)
    {
        dest[i] = source[i];
    }
    dest[i] = '\0';
}

int to_number(char *string)
{
    int num = 0;

    for(int i = 0; string[i] != '\0'; i++)
    {
        num = num * 10 + string[i] - '0';
    }
    return num;
}

int init_fen(struct fen *fn, char *fen_string)
{
    char *strings[6];
    for(int string = 0; string < 6; string++)
    {
        strings[string] = NULL;
    }

    int string_count = 0;
    int start = 0;
    int i;

    //tokenizing string
    for(i = 0; fen_string[i] != '\0'; i++)
    {
        if(fen_string[i] == ' ')
        {
            strings[string_count] = (char *)malloc(sizeof(char) * (i - start) + 1);
            if(strings[string_count] == NULL)
            {
                free_strings(strings, 6);
                return 0;
            }
            copy_string_range(strings[string_count], fen_string, start, i - 1);
            string_count++;
            start = i + 1;
        }
    }
    
    strings[string_count] = (char *)malloc(sizeof(char) * (i - start) + 1);
    if(strings[string_count] == NULL)
    {
        free_strings(strings, 6);
        return 0;
    }

    copy_string_range(strings[string_count], fen_string, start, i - 1);
    string_count++;
    
    if(string_count != 6)
    {
        free_strings(strings, 6);
        return 0;
    }

    string_cpy(fn->piece_placement, strings[0]);

    char turn = strings[1][0];
    
    if(turn == 'b')
    {
        fn->turn = BLACK;
    }
    else
    {
        fn->turn = WHITE;
    }

    if(strings[2][0] == '-')
    {
        fn->black_castle = fn->white_castle = 0;
    }
    else
    {
        char *castling = strings[2];
        int black_castle = 0, white_castle = 0;
        for( i = 0; castling[i] != '\0'; i++)
        {
            if(castling[i] == 'K')
            {
                white_castle = white_castle | 2;
            }
            else if(castling[i] == 'Q')
            {
                white_castle = white_castle | 1;
            }
            else if(castling[i] == 'k')
            {
                black_castle = black_castle | 2;
            }
            else if(castling[i] == 'q')
            {
                black_castle = black_castle | 1;
            }
        }
        fn->black_castle = black_castle;
        fn->white_castle = white_castle;
    }

    if(strings[3][0] == '-')
    {
        fn->en_passant = -1;
    }
    else
    {
        fn->en_passant = en_passant_position(strings[3]);
    }
    fn->half_moves = to_number(strings[4]);
    fn->full_moves = to_number(strings[5]);

    free_strings(strings, 6);
    return 1;
}

void init_chess_game(struct chess_game *game, struct fen *fn)
{
    game->turn = fn->turn;
    game->white_castle = fn->white_castle;
    game->black_castle = fn->black_castle;
    generate_steps_to_edges(game->distance_to_borders);
    init_board_from_fen(game->board, fn->piece_placement);
    init_piece_list(game);
    generate_fen(game);
    if(fn->en_passant == -1)
    {
        game->last_move = NULL;
    }
    else
    {
        int back_direction;
        int front_direction;
        if(game->turn == BLACK)
        {
            back_direction = N;
            front_direction = S;
        }
        else
        {
            back_direction = S;
            front_direction = N;
        }

        game->last_move = init_move(fn->en_passant + back_direction, fn->en_passant + front_direction, DOUBLE_PAWN_PUSH);
    }
}

void make_move(struct chess_game *game, struct move *mv)
{
    int *board = game->board;
    game->last_move = mv;
    int src = mv->src;
    int dest = mv->dest;
    int turn = game->turn;

    struct piece_list *turn_piece_list, *opposite_piece_list;
    if(turn == WHITE)
    {
        turn_piece_list = &game->white_piece_list;
        opposite_piece_list = &game->black_piece_list;
    }
    else
    {
        turn_piece_list = &game->black_piece_list;
        opposite_piece_list = &game->white_piece_list;
    }

    int move_type = mv->type;
    if(move_type == QUIET_MOVE || move_type == DOUBLE_PAWN_PUSH)
    {
        update_piece_index(turn_piece_list, piece_type(board[src]), mv);
        board[dest] = board[src];
        board[src] = 0;
    }
    else if(move_type == CAPTURES)
    {
        add_captured_piece(&game->captured_piece_list, board[dest]);
        remove_piece_index(opposite_piece_list, piece_type(board[dest]), dest);
        update_piece_index(turn_piece_list, piece_type(board[src]), mv);
        board[dest] = board[src];
        board[src] = 0;
    }
    else if(move_type == ENPASSANT_CAPTURE)
    {
        if(turn == BLACK)
        {
            add_captured_piece(&game->captured_piece_list, board[dest + N]);
            remove_piece_index(opposite_piece_list, PAWN, dest + N);
            board[dest + N] = 0;
        }
        else
        {
            add_captured_piece(&game->captured_piece_list, board[dest + S]);
            remove_piece_index(opposite_piece_list, PAWN, dest + S);
            board[dest + S] = 0;
        }
        update_piece_index(turn_piece_list, piece_type(board[src]), mv);
        board[dest] = board[src];
        board[src] = 0;
    }
    else if(move_type == KING_CASTLE)
    {
        int king = board[src];
        int rook = board[src + 3 * E];
        update_piece_index(turn_piece_list, KING, mv);
        struct move rook_move = {src + 3 * E, dest + W};
        update_piece_index(turn_piece_list, ROOK, &rook_move);
        board[dest] = king;
        board[src] = 0;
        board[dest + W] = rook;
        board[src + 3 * E] = 0;
    }
    else if(move_type == QUEEN_CASTLE)
    {
        int king = board[src];
        int rook = board[src + 4 * W];
        update_piece_index(turn_piece_list, KING, mv);
        struct move rook_move = {src + 4 * W, dest + E};
        update_piece_index(turn_piece_list, ROOK, &rook_move);
        board[dest] = king;
        board[src] = 0;
        board[dest + E] = rook;
        board[src + 4 * W] = 0;
    }
    else if((move_type & 8) == 8)
    {
        if((move_type & 4) == 4)//captured promotion
        {
            move_type = move_type & 11;//setting capture bit to 0
            add_captured_piece(&game->captured_piece_list, board[dest]);
            remove_piece_index(opposite_piece_list, piece_type(board[dest]), dest);
        }
        remove_piece_index(turn_piece_list, PAWN, src);
        board[src] = 0;
        if(move_type == QUEEN_PROMOTION)
        {
            add_piece_index(turn_piece_list, QUEEN, dest);
            board[dest] = turn | QUEEN;
        }
        else if(move_type == KNIGHT_PROMOTION)
        {
            add_piece_index(turn_piece_list, KNIGHT, dest);
            board[dest] = turn | KNIGHT;
        }
        else if(move_type == ROOK_PROMOTION)
        {
            add_piece_index(turn_piece_list, ROOK, dest);
            board[dest] = turn | ROOK;
        }
        else
        {
            add_piece_index(turn_piece_list, BISHOP, dest);
            board[dest] = turn | BISHOP;
        }
    }
}

int main()
{
    struct chess_game game;
    // char fen_string[] =  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq";
    char fen_string[] = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3";

    struct fen fn;

    init_fen(&fn, fen_string);
    init_chess_game(&game, &fn);
    display_name_board(game.board);
    printf("%s\n", game.fen);
    display_piece_list(&game.white_piece_list);
    // display_piece_list(&game.black_piece_list);
    struct queue* q = generate_moves(&game);
    // display_queue(q);
    free_queue(q);
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