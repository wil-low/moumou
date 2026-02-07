#include "ai.h"
#include "core.h"
#include <stdlib.h>

uint8_t ai_move(GameState *state) {
    switch (state->_players[state->_cur_player]._level) {
    case Level_2:
        if (state->_valid_moves._count)
            return rand() % state->_valid_moves._count;
        if (state->_valid_moves._draw)
            return CMD_DRAW;
        if (state->_valid_moves._pass)
            return CMD_PASS;
        break;
    default: // Level_1
        if (state->_valid_moves._count)
            return 0;
        if (state->_valid_moves._pass)
            return CMD_PASS;
        if (state->_valid_moves._draw)
            return CMD_DRAW;
        break;
    }
    return 0; // never reach here
}

Suit ai_demand_suit(GameState *state) {
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
        return (Suit)max_idx;
    } break;
    default: // Level_1
        return rand() % SuitCount;
    }
    return (Suit)Undefined; // never reach here
}
