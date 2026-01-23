#ifndef _CORE_H_
#define _CORE_H_

#include <stdbool.h>
#include <stdint.h>

#define Undefined ' '

typedef enum : uint8_t {
    Spades = 's',
    Hearts = 'h',
    Diamonds = 'd',
    Clubs = 'c'
} Suit;

typedef enum : uint8_t {
    Six = '6',
    Seven = '7',
    Eight = '8',
    Nine = '9',
    Ten = 'T',
    Jack = 'J',
    Queen = 'Q',
    King = 'K',
    Ace = 'A'
} Value;

static const Suit suits[] = {Spades, Hearts, Diamonds, Clubs};
static const Value values[] = {Six,  Seven, Eight, Nine, Ten,
                               Jack, Queen, King,  Ace};

typedef struct {
    Value _value;
    Suit _suit;
} Card;

typedef struct {
    uint8_t _count;
    Card _items[9 * 4];
} Deck;

typedef enum : uint8_t {
    Human = 1 << 0,
    Dealt = 1 << 1,
} Flag;

#define MAX_CARDS_IN_HAND 16
#define CMD_DRAW 253
#define CMD_PASS 254
#define CMD_REPEAT_FIND 255

#define PLAY_OK 0
#define PLAY_DEMAND_SUIT 1
#define PLAY_OPPONENT_SKIPS 2
#define PLAY_MOUMOU 3

#define PLAYER_COUNT 2
#define INITIAL_HAND 5

typedef struct {
    uint8_t _flags;
    Deck _hand;
    uint8_t _score;
} Player;

typedef struct {
    uint8_t _items[MAX_CARDS_IN_HAND];
    uint8_t _count;
    bool _draw;
    bool _pass;
} ValidMoves;

typedef struct {
    Player _players[PLAYER_COUNT];
    Deck _deck;
    Deck _table;
    Deck _played;
    uint8_t _cur_player;
    uint8_t _turn;
    Suit _demanded;
    ValidMoves _valid_moves;
    Card _last_card;
    uint8_t _moumou_counter;
} GameState;

void recycle_deck(GameState *state);
void move_played_to_table(GameState *state);
Card deal(GameState *state);
bool draw(GameState *state, uint8_t player_idx, uint8_t count);
bool find_valid_moves(GameState *state, uint8_t player_idx);
uint8_t play_card(GameState *state, uint8_t player_idx, uint8_t card_idx);
void new_round(GameState *state);
void calculate_scores(GameState *state);

uint8_t input_move(GameState *state);
Suit input_suit(GameState *);
void input_wait(const char *message);

#endif
