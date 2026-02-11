#include "ai.h"
#include "core.h"
#include <stdlib.h>

Command ai_move(GameState *state) {
    switch (state->_players[state->_cur_player]._level) {
    case Level_2:
        if (state->_valid_moves._count)
            return state->_valid_moves
                ._items[rand() % state->_valid_moves._count];
        if (state->_valid_moves._flags & FLAG_DRAW)
            return CMD_DRAW;
        if (state->_valid_moves._flags & FLAG_PASS)
            return CMD_PASS;
        break;
    case Level_1:
        if (state->_valid_moves._count)
            return state->_valid_moves._items[0];
        if (state->_valid_moves._flags & FLAG_PASS)
            return CMD_PASS;
        if (state->_valid_moves._flags & FLAG_DRAW)
            return CMD_DRAW;
        break;
    }
    return CMD_NONE; // Human
}

Command ai_demand_suit(GameState *state) {
    Player *p = &state->_players[state->_cur_player];
    switch (p->_level) {
    case Level_2: {
        uint8_t count_by_suit[SuitCount] = {0, 0, 0, 0};
        for (uint8_t i = 0; i < p->_hand._count; ++i) {
            count_by_suit[CardSuit(p->_hand._items[i])]++;
        }
        uint8_t max_idx = 0;
        for (uint8_t i = 1; i < SuitCount; ++i) {
            if (count_by_suit[i] > count_by_suit[max_idx])
                max_idx = i;
        }
        return CMD_DEMAND_SPADES + max_idx;
    }
    case Level_1:
        return CMD_DEMAND_SPADES + rand() % SuitCount;
    }
    return CMD_SELECT_SUIT; // Human
}
