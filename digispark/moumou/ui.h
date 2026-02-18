#ifndef _UI_H_
#define _UI_H_

#include "core.h"

void render_card(Card *card);
void render_player(GameState *state, uint8_t idx);
void render_table(GameState *state);
void render_game(GameState *state);

uint8_t human_move(GameState *state);
Suit human_demand_suit(GameState *);

#endif
