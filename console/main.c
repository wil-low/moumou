#include "config.h"
#include "core.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

GameState game_state;

void setup() {
    printf("Moumou\n");

    srand(RANDOM_SEED ? RANDOM_SEED : (unsigned int)time(NULL));

    GameState *state = &game_state;

    for (uint8_t i = 0; i < PLAYER_COUNT; ++i)
        state->_players[i]._score = 0;
    state->_players[0]._level = PLAYER0_LEVEL;
    state->_players[1]._level = PLAYER1_LEVEL;

    new_round(state);
}

void loop() {
    GameState *state = &game_state;
    bool has_cards = find_valid_moves(state, state->_cur_player);
    render_game(state);

    if (!has_cards) {
        printf("Player #%d has no cards\n", state->_cur_player);
        update_score(state);
        new_round(state);
        return;
    }

    uint8_t card_idx = input_move(state);

    if (card_idx == CMD_DRAW) {
        draw(state, state->_cur_player, 1);
        state->_valid_moves._draw = false;
        printf("Player #%d draws\n", state->_cur_player);
    } else if (card_idx == CMD_PASS) {
        printf("Player #%d passes\n", state->_cur_player);
    } else {
        card_idx = state->_valid_moves._items[card_idx];
        uint8_t result = play_card(state, state->_cur_player, card_idx);
        if (result == PLAY_MOUMOU) {
            printf("Player #%d declares Moumou\n", state->_cur_player);
            update_score(state);
            new_round(state);
            return;
        } else {
            state->_demanded = Undefined;
            state->_valid_moves._restrict_value =
                result != PLAY_OPPONENT_SKIPS &&
                CardValue(state->_last_card) != Six;
        }
    }
    state->_valid_moves._pass = CardValue(state->_last_card) != Six;

    if (card_idx == CMD_PASS) {
        if (state->_played._count && CardValue(state->_last_card) == Jack) {
            state->_demanded = Undefined;
            state->_demanded = input_suit(state);
            printf("Player #%d demands %c\n", state->_cur_player,
                   SUITS[state->_demanded]);
        }
        move_played_to_table(state);
        state->_valid_moves._pass = false;
        state->_valid_moves._restrict_value = false;
        state->_turn++;
        state->_cur_player = (state->_cur_player + 1) % PLAYER_COUNT;
        printf("\n======================== Player %d, turn %d "
               "========================\n\n",
               state->_cur_player, state->_turn);
    } else {
        // printf("=====\n\n");
    }
}

int main(/*int argc, char **argv*/) {
    setup();

    while (true)
        loop();

    return 0;
}
