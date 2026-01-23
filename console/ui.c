#include "core.h"
#include <stdio.h>
#include <string.h>

void render_card(Card *card) {
    printf("%c%c ", VALUES[card->_value], SUITS[card->_suit]);
}

void render_player(GameState *state, uint8_t idx) {
    Player *p = &state->_players[idx];

    if (p->_level == Human)
        printf("Player %d (Human), cards %d, score %d\n", idx, p->_hand._count,
               p->_score);
    else
        printf("Player %d (Bot Lev.%d), cards %d, score %d\n", idx, p->_level,
               p->_hand._count, p->_score);

    if (p->_level == Human || BOT_CARDS_VISIBLE) {
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
    }
    printf("\n\n");
}

void render_table(GameState *state) {
    printf("Table");
    if (state->_demanded != Undefined)
        printf(", demanded: %c", SUITS[state->_demanded]);
    printf("\n  > ");
    uint8_t start =
        state->_table._count > SuitCount ? state->_table._count - SuitCount : 0;
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
    printf("\n======================== Player %d, turn %d "
           "========================\n\n",
           state->_cur_player, state->_turn);

    if (PLAYER0_LEVEL == Human && PLAYER1_LEVEL == Human) {
        // rotate table
        render_player(state, 1 - state->_cur_player);
        render_table(state);
        render_player(state, state->_cur_player);
    } else {
        render_player(state, 1);
        render_table(state);
        render_player(state, 0);
    }
}

uint8_t human_move(GameState *state) {
    int result;
    while (true) {
        printf("Enter your choice: ");
        char cmd[5];
        scanf("%s", cmd);
        if (strcmp(cmd, "d") == 0 && state->_valid_moves._draw)
            return CMD_DRAW;
        if (strcmp(cmd, "p") == 0 && state->_valid_moves._pass)
            return CMD_PASS;
        if (strcmp(cmd, "r") == 0)
            return CMD_REPEAT_FIND;
        if (sscanf(cmd, "%d", &result) == 1) {
            if (result >= 0 && result < state->_valid_moves._count)
                return result;
        }
    }
    return 0; // never reach here
}

Suit human_demand_suit(GameState *) {
    while (true) {
        printf("Demand suit (%s): ", SUITS);
        char suit[5];
        scanf("%s", suit);
        for (uint8_t i = 0; i < SuitCount; ++i) {
            if (suit[0] == SUITS[i])
                return (Suit)i;
        }
    }
    return (Suit)Undefined;
}
