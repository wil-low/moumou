#ifndef _CORE_H_
#define _CORE_H_

#include "config.h"
#include <stdbool.h>
#include <stdint.h>

#define Undefined UINT8_MAX

typedef enum : uint8_t {
    Spades = 0,
    Hearts,
    Diamonds,
    Clubs,
    SuitCount
} Suit;

typedef enum : uint8_t {
    Six = 0,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace,
    ValueCount
} Value;

static const char SUITS[] = "shdc";
static const char VALUES[] = "6789TJQKA";

typedef struct {
    Value _value;
    Suit _suit;
} Card;

typedef struct {
    uint8_t _count;
    Card _items[SuitCount * ValueCount];
} Deck;

typedef enum : uint8_t {
    Human = 0,
    Level_1,
    Level_2,
} AILevel;

#define CMD_DRAW 253
#define CMD_PASS 254

#define PLAY_OK 0
// #define PLAY_DEMAND_SUIT 1
#define PLAY_OPPONENT_SKIPS 1
#define PLAY_MOUMOU 2

typedef struct {
    AILevel _level;
    Deck _hand;
    uint16_t _score;
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
