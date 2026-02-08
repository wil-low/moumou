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

#define CMD_DRAW 253
#define CMD_PASS 254
#define CMD_NONE 255

#define PLAY_OK 0
#define PLAY_OPPONENT_SKIPS 1
#define PLAY_MOUMOU 2

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
    bool _draw;
    bool _pass;
    bool _restrict_value;
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
    uint8_t _input_cmd;
};

void recycle_deck(GameState *state);
void move_played_to_table(GameState *state);
Card deal(GameState *state);
bool draw(GameState *state, uint8_t player_idx, uint8_t count);
bool find_valid_moves(GameState *state, uint8_t player_idx);
uint8_t play_card(GameState *state, uint8_t player_idx, uint8_t card_idx);
void new_round(GameState *state);
uint16_t hand_score(GameState *state, uint8_t player_idx);
void update_score(GameState *state);
void deal_card(GameState *state, uint8_t player_idx, Value value, Suit suit);

uint8_t input_move(GameState *state);
Suit input_suit(GameState *);
void input_wait(const char *message);

#endif
