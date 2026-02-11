#ifndef _CORE_H_
#define _CORE_H_

#include "config.h"
#include "pile.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    Human = 0,
    Level_1,
    Level_2,
} AILevel;

typedef enum {
    MODE_PLAYER_MOVE = 0,
    MODE_ANIMATE,
    MODE_DEMAND_SUIT,
    MODE_ROUND_OVER
} GameMode;

typedef enum {
    CMD_PLAY_CARD = 0,
    CMD_DRAW = 100,
    CMD_PASS = 101,
    CMD_NEXT_PLAYER = 102,
    CMD_NEW_ROUND = 103,
    CMD_DEMAND_SPADES = 104,
    CMD_DEMAND_HEARTS = 105,
    CMD_DEMAND_DIAMOND = 106,
    CMD_DEMAND_CLUBS = 107,
    CMD_NONE = 255
} Command;

const uint8_t FLAG_DRAW = 1 << 0;
const uint8_t FLAG_PASS = 1 << 1;
const uint8_t FLAG_RESTRICT_VALUE = 1 << 2;
const uint8_t FLAG_RECYCLE_DECK = 1 << 3;
const uint8_t FLAG_DEMAND_SUIT = 1 << 4;
const uint8_t FLAG_MOUMOU = 1 << 5;
const uint8_t FLAG_EMPTY_HAND = 1 << 6;

const uint8_t PLAY_OK = 0;
const uint8_t PLAY_OPPONENT_SKIPS = 1;

class Player {
  public:
    AILevel _level;
    Pile _hand;
    uint16_t _score;
};

class ValidMoves {
  public:
    uint8_t _items[MAX_CARDS_IN_HAND];
    uint8_t _count;
    uint8_t _flags;
};

class GameState {
  public:
    Player _players[PLAYER_COUNT];
    Pile _deck;
    Pile _table;
    Pile _played;
    uint8_t _cur_player;
    uint8_t _turn;
    Suit _demanded;
    ValidMoves _valid_moves;
    Card _last_card;
    uint8_t _moumou_counter;
    Command _pending_cmd;
    Command _input_cmd;
    uint8_t _fvm_calls;
};

class UI;

void process_input(GameState *state, UI *ui);
void recycle_deck(GameState *state);
void move_played_to_table(GameState *state);
Card deal(GameState *state);
bool draw(GameState *state, uint8_t player_idx, uint8_t count);
bool find_valid_moves(GameState *state, uint8_t player_idx);
void animation_complete(GameState *gameState);
uint8_t play_card(GameState *state, uint8_t player_idx, uint8_t card_idx);
uint8_t opponent_draws(Card *p_card);
void new_round(GameState *state, UI *ui);
uint16_t hand_score(GameState *state, uint8_t player_idx);
void update_score(GameState *state);
void deal_card(GameState *state, uint8_t player_idx, Value value, Suit suit);

#endif
