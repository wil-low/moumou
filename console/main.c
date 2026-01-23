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

    srand(RANDOM_SEED ? RANDOM_SEED : time(NULL));

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
        calculate_scores(state);
        new_round(state);
        return;
    }

    uint8_t card_idx = input_move(state);

    if (card_idx == CMD_DRAW) {
        draw(state, state->_cur_player, 1);
        state->_valid_moves._draw = false;
    } else if (card_idx == CMD_PASS) {
    } else if (card_idx == CMD_REPEAT_FIND) {
    } else {
        card_idx = state->_valid_moves._items[card_idx];
        uint8_t result = play_card(state, state->_cur_player, card_idx);
        state->_demanded = Undefined;
        if (result == PLAY_DEMAND_SUIT) {
            state->_demanded = input_suit(state);
        } else if (result == PLAY_OPPONENT_SKIPS) {
            move_played_to_table(state);
        } else if (result == PLAY_MOUMOU) {
            printf("Player #%d declares Moumou\n", state->_cur_player);
            calculate_scores(state);
            new_round(state);
            return;
        }
    }
    state->_valid_moves._pass = state->_last_card._value != Six;

    if (card_idx == CMD_PASS) {
        move_played_to_table(state);
        state->_valid_moves._pass = false;
        state->_turn++;
        state->_cur_player = (state->_cur_player + 1) % PLAYER_COUNT;
    }
}

int main(/*int argc, char **argv*/) {
    setup();

    while (true)
        loop();

    return 0;
}
