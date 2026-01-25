#include "core.h"
#include "ai.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void recycle_deck(GameState *state) {
    state->_deck._count = state->_table._count - 3;
    for (uint8_t i = 0; i < state->_deck._count; ++i) {
        state->_deck._items[i] = state->_table._items[i];
    }
    for (uint8_t i = 0; i < 3; ++i) {
        state->_table._items[i] = state->_table._items[state->_deck._count + i];
    }
    state->_table._count = 3;
}

void move_played_to_table(GameState *state) {
    for (uint8_t i = 0; i < state->_played._count; ++i) {
        state->_table._items[state->_table._count++] = state->_played._items[i];
    }
    state->_played._count = 0;
    state->_valid_moves._draw = true;
}

Card deal(GameState *state) {
    if (state->_deck._count > 0) {
        int idx = rand() % state->_deck._count;
        state->_deck._count--;
        Card tmp = state->_deck._items[idx];
        state->_deck._items[idx] = state->_deck._items[state->_deck._count];
        return tmp;
    }
    recycle_deck(state);
    return deal(state);
}

bool draw(GameState *state, uint8_t player_idx, uint8_t count) {
    Player *p = &state->_players[player_idx];
    for (uint8_t i = 0; i < count; i++) {
        if (p->_hand._count < MAX_CARDS_IN_HAND) {
            Card card = deal(state);
            p->_hand._items[p->_hand._count] = card;
            p->_hand._count++;
        }
    }
    return true;
}

bool find_valid_moves(GameState *state, uint8_t player_idx) {
    Player *p = &state->_players[player_idx];
    if (p->_hand._count == 0)
        return false;

    state->_valid_moves._count = 0;

    for (uint8_t i = 0; i < p->_hand._count; i++) {
        Card *p_card = &p->_hand._items[i];
        if (state->_played._count) {
            if (state->_last_card._value == Six) {
                if (p_card->_value != Jack &&
                    p_card->_value != state->_last_card._value &&
                    p_card->_suit != state->_last_card._suit)
                    continue;
            } else if (p_card->_value != state->_last_card._value)
                continue;
        } else {
            if (p_card->_value == Jack || p_card->_value == Six) {
            } else if (state->_demanded != Undefined) {
                if (p_card->_suit != state->_demanded)
                    continue;
            } else if (state->_last_card._value != Undefined &&
                       (p_card->_value == state->_last_card._value ||
                        p_card->_suit == state->_last_card._suit)) {
            } else {
                continue;
            }
        }
        state->_valid_moves._items[state->_valid_moves._count++] = i;
    }
    if (state->_last_card._value == Six) {
        state->_valid_moves._draw = true;
        state->_valid_moves._pass = false;
    }
    if (state->_valid_moves._count == 0 && !state->_valid_moves._draw)
        state->_valid_moves._pass = true;
    return true;
}

uint8_t play_card(GameState *state, uint8_t player_idx, uint8_t card_idx) {
    Player *p = &state->_players[player_idx];
    Card *p_card = &p->_hand._items[card_idx];
    // apply effects
    uint8_t result = PLAY_OK;
    uint8_t next_player = (player_idx + 1) % PLAYER_COUNT;
    if (p_card->_value == Eight) {
        draw(state, next_player, 2);
        result = PLAY_OPPONENT_SKIPS;
    }
    if (p_card->_value == Ace) {
        result = PLAY_OPPONENT_SKIPS;
    }
    if (p_card->_value == Seven) {
        draw(state, next_player, 1);
    }
    if (p_card->_suit == Clubs && p_card->_value == King) {
        draw(state, next_player, 5);
        result = PLAY_OPPONENT_SKIPS;
    }

    // check moumou
    if (p_card->_value == state->_last_card._value) {
        ++state->_moumou_counter;
        if (state->_moumou_counter == SuitCount)
            result = PLAY_MOUMOU;
    } else
        state->_moumou_counter = 1;

    state->_valid_moves._draw =
        p_card->_value == Six || result == PLAY_OPPONENT_SKIPS;
    state->_valid_moves._pass = p_card->_value != Six;

    state->_last_card = *p_card;
    state->_played._items[state->_played._count++] = *p_card;
    for (uint8_t i = card_idx + 1; i < p->_hand._count; ++i) {
        p->_hand._items[i - 1] = p->_hand._items[i];
    }
    p->_hand._count--;

    printf("Player #%d plays ", state->_cur_player);
    render_card(&state->_last_card);
    printf("\n");

    return result;
}

void new_round(GameState *state) {
    for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
        state->_players[i]._hand._count = 0;
    }

    state->_cur_player = 0;
    state->_turn = 0;
    state->_demanded = Undefined;
    state->_valid_moves._draw = false;
    state->_valid_moves._pass = false;
    state->_last_card._suit = Undefined;
    state->_last_card._value = Undefined;
    state->_moumou_counter = 0;

    // Init deck
    state->_deck._count = SuitCount * ValueCount;
    state->_table._count = 0;
    state->_played._count = 0;
    uint8_t idx = 0;
    for (uint8_t s = 0; s < SuitCount; ++s) {
        for (uint8_t v = 0; v < ValueCount; ++v) {
            state->_deck._items[idx]._value = v;
            state->_deck._items[idx]._suit = s;
            ++idx;
        }
    }

    printf("\n======================== New round, Player %d, turn %d "
           "========================\n\n",
           state->_cur_player, state->_turn);

    // Deal initial cards
    draw(state, 0, INITIAL_HAND);
    draw(state, 1, INITIAL_HAND);
    play_card(state, 0, 0);
}

void calculate_scores(GameState *state) {
    for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
        Player *p = &state->_players[i];
        for (uint8_t idx = 0; idx < p->_hand._count; ++idx) {
            Value val = p->_hand._items[idx]._value;
            switch (val) {
            case Ten:
            case Queen:
            case King:
                p->_score += 10;
                break;
            case Ace:
                p->_score += 15;
                break;
            case Jack:
                p->_score += 20;
                break;
            default:
                p->_score += val - Six + 6;
                break;
            }
        }
        printf("Player #%d score: %d\n", i, p->_score);
    }

    input_wait("\nPress Enter to start a new round...");
}

uint8_t input_move(GameState *state) {
    if (state->_players[state->_cur_player]._level == Human)
        return human_move(state);
    return ai_move(state);
}

Suit input_suit(GameState *state) {
    if (state->_players[state->_cur_player]._level == Human)
        return human_demand_suit(state);
    return ai_demand_suit(state);
}

void input_wait(const char *message) {
    printf("%s", message);
    /* flush pending input */
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
    /* wait for Enter */
    getchar();
}
