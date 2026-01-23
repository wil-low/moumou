#include "core.h"
#include <stdio.h>

void render_card(Card *card) {
    printf("%c%c ", card->_value, card->_suit);
}

void render_player(GameState *state, uint8_t idx) {
    Player *p = &state->_players[idx];
    printf("Player #%d (%s), cards %d, score %d\n", idx,
           (p->_flags & Human) ? "human" : "robot", p->_hand._count, p->_score);
    printf("    ");
    for (uint8_t i = 0; i < p->_hand._count; ++i) {
        render_card(&p->_hand._items[i]);
    }
    printf("  Draw   Pass");
    if (idx == state->_cur_player) {
        // show valid moves
        printf("\n");
        printf("    ");
        uint8_t idx = 0;
        for (uint8_t i = 0; i < p->_hand._count; ++i) {
            if (idx < state->_valid_moves._count &&
                i == state->_valid_moves._items[idx]) {
                printf("%2d ", idx);
                idx++;
            } else
                printf("   ");
        }
        if (state->_valid_moves._draw)
            printf("     d");
        else
            printf("      ");
        if (state->_valid_moves._pass)
            printf("      p");
    } else {
        printf("\n");
    }
    printf("\n\n");
}

void render_table(GameState *state) {
    printf("Table");
    if (state->_demanded != Undefined)
        printf(", demanded: %c", state->_demanded);
    printf("\n  > ");
    uint8_t start = state->_table._count > 4 ? state->_table._count - 4 : 0;
    for (uint8_t i = start; i < state->_table._count; ++i) {
        render_card(&state->_table._items[i]);
    }
    printf("\nPlayed\n  > ");
    for (uint8_t i = 0; i < state->_played._count; ++i) {
        render_card(&state->_played._items[i]);
    }
    printf("\n\n");
}

void render_game(GameState *state) {
    printf("\n======================== Turn %d ========================\n\n",
           state->_turn);
    render_player(state, 1 - state->_cur_player);
    render_table(state);
    render_player(state, state->_cur_player);
}
