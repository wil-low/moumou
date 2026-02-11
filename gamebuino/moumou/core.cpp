#include "core.h"
#include "ai.h"
#include "ui.h"

void new_round(GameState *state, UI *ui) {
    ui->_activeLocation = stock;
    ui->_cardIndex = 0;
    ui->_cursorX = 0;
    ui->_cursorY = 4;

    state->_deck.newDeck();
    state->_deck.shuffle();

    state->_players[0]._hand.empty();
    state->_players[1]._hand.empty();
    state->_cur_player = 0;
    state->_turn = 0;
    state->_demanded = Undefined;
    state->_valid_moves._flags = FLAG_DRAW;
    state->_last_card = Card();
    state->_moumou_counter = 0;
    state->_pending_cmd = CMD_NONE;
    state->_fvm_calls = 0;

    state->_played.empty();
    state->_table.empty();

    state->_players[1]._level = PLAYER1_LEVEL; // ui->_botLevel;

    // Initialize the data structure to deal out the initial board.
    ui->_cardAnimationCount = 0;
    for (int i = 0; i < INITIAL_HAND; i++)
        for (int p = 0; p < 2; p++)
            ui->animateMove(&state->_deck, 0, &state->_players[p]._hand, i);
    ui->animateMove(&state->_deck, 0, &state->_table, 0);

    ui->_dealingCount = ui->_cardAnimationCount;
    ui->_cardAnimationCount = 0;
    ui->_mode = MODE_ANIMATE;
}

bool find_valid_moves(GameState *state, uint8_t player_idx) {
    state->_fvm_calls++;
    state->_valid_moves._count = 0;

    Player *p = &state->_players[player_idx];
    if (p->_hand._count == 0) {
        if (state->_played._count && (CardValue(state->_last_card) == Six ||
                                      CardValue(state->_last_card) == Ace)) {
            // cannot finish with Ace or 6
            state->_valid_moves._flags |= FLAG_DRAW;
            state->_valid_moves._flags &= ~FLAG_PASS;
            return true;
        }
        return false;
    }

    for (uint8_t i = 0; i < p->_hand._count; i++) {
        Card *p_card = &p->_hand._items[i];
        bool allowed = true;
        if ((state->_valid_moves._flags & FLAG_RESTRICT_VALUE) &&
            CardValue(*p_card) != CardValue(state->_last_card)) {
            allowed = false;
        } else if (CardValue(*p_card) != Jack && CardValue(*p_card) != Six) {
            if (CardValue(*p_card) != CardValue(state->_last_card)) {
                if (state->_demanded != Undefined)
                    allowed = CardSuit(*p_card) == state->_demanded;
                else
                    allowed = CardSuit(*p_card) == CardSuit(state->_last_card);
            }
        }
        if (allowed)
            state->_valid_moves._items[state->_valid_moves._count++] = i;
    }
    if (CardValue(state->_last_card) == Six) {
        state->_valid_moves._flags |= FLAG_DRAW;
        state->_valid_moves._flags &= ~FLAG_PASS;
    }
    if (state->_valid_moves._count == 0 &&
        !(state->_valid_moves._flags & FLAG_DRAW))
        state->_valid_moves._flags |= FLAG_PASS;
    return true;
}

void animation_complete(GameState *state) {
    state->_input_cmd = CMD_NONE;
    find_valid_moves(state, state->_cur_player);
    state->_pending_cmd = ai_move(state);
    //  process_input(&state, this);
}

void process_input(GameState *state, UI *ui) {
    if (state->_input_cmd != CMD_NONE) {
        if (state->_input_cmd == CMD_DRAW) {
            state->_valid_moves._flags &= ~FLAG_DRAW;
        } else if (state->_input_cmd == CMD_PASS) {
        } else {
            uint8_t result =
                play_card(state, state->_cur_player, state->_input_cmd);
            if (state->_valid_moves._flags & FLAG_MOUMOU) {
                // printf("Player #%d declares Moumou\n",
                //        state->_cur_player);
                update_score(state);
                new_round(state, ui);
                return;
            } else {
                state->_demanded = Undefined;
                if (result != PLAY_OPPONENT_SKIPS &&
                    CardValue(state->_last_card) != Six)
                    state->_valid_moves._flags |= FLAG_RESTRICT_VALUE;
                else
                    state->_valid_moves._flags &= ~FLAG_RESTRICT_VALUE;
            }
        }
        if (CardValue(state->_last_card) != Six)
            state->_valid_moves._flags |= FLAG_PASS;
        else
            state->_valid_moves._flags &= ~FLAG_PASS;

        if (state->_input_cmd == CMD_PASS) {
            if (/*state->_played._count &&*/ CardValue(state->_last_card) ==
                Jack) {
                state->_demanded = Undefined;
                // state->_demanded = input_suit(state);
                //  printf("Player #%d demands %c\n", state->_cur_player,
                //         SUITS[state->_demanded]);
            }
            // move_played_to_table(state);
            state->_valid_moves._flags &= ~FLAG_PASS;
            state->_valid_moves._flags &= ~FLAG_RESTRICT_VALUE;
            state->_turn++;
            state->_cur_player = (state->_cur_player + 1) % PLAYER_COUNT;
        }
        state->_input_cmd = CMD_NONE;
        // ui->turnLoop();
    }
}

uint8_t opponent_draws(Card *p_card) {
    if (CardValue(*p_card) == Eight)
        return 2;
    if (CardValue(*p_card) == Seven)
        return 1;
    if (CardSuit(*p_card) == Clubs && CardValue(*p_card) == King)
        return 5;
    return 0;
}

uint8_t play_card(GameState *state, uint8_t player_idx, uint8_t card_idx) {
    Player *p = &state->_players[player_idx];
    Card *p_card = &p->_hand._items[card_idx];
    // apply effects
    uint8_t result = PLAY_OK;
    if (CardValue(*p_card) == Eight) {
        result = PLAY_OPPONENT_SKIPS;
    }
    if (CardValue(*p_card) == Ace) {
        result = PLAY_OPPONENT_SKIPS;
    }
    if (CardSuit(*p_card) == Clubs && CardValue(*p_card) == King) {
        result = PLAY_OPPONENT_SKIPS;
    }
    // check moumou
    if (CardValue(*p_card) == CardValue(state->_last_card)) {
        ++state->_moumou_counter;
        if (state->_moumou_counter == SuitCount)
            state->_valid_moves._flags |= FLAG_MOUMOU;
    } else
        state->_moumou_counter = 1;

    if (CardValue(*p_card) == Six || result == PLAY_OPPONENT_SKIPS)
        state->_valid_moves._flags |= FLAG_DRAW;
    else
        state->_valid_moves._flags &= ~FLAG_DRAW;
    if (CardValue(*p_card) != Six)
        state->_valid_moves._flags |= FLAG_PASS;
    else
        state->_valid_moves._flags &= ~FLAG_PASS;

    return result;
}

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
    // state->_valid_moves._flags |= FLAG_DRAW;
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

void deal_card(GameState *state, uint8_t player_idx, Value value, Suit suit) {
    Card card(value, suit, false);
    Player *p = &state->_players[player_idx];
    p->_hand._items[p->_hand._count] = card;
    p->_hand._count++;
}

uint16_t hand_score(GameState *state, uint8_t player_idx) {
    uint16_t score = 0;
    Player *p = &state->_players[player_idx];
    for (uint8_t idx = 0; idx < p->_hand._count; ++idx) {
        Value val = CardValue(p->_hand._items[idx]);
        switch (val) {
        case Ten:
        case Queen:
        case King:
            score += 10;
            break;
        case Ace:
            score += 15;
            break;
        case Jack:
            score += 20;
            break;
        default:
            break;
        }
    }
    return score;
}

void update_score(GameState *state) {
    for (uint8_t i = 0; i < PLAYER_COUNT; ++i) {
        Player *p = &state->_players[i];
        p->_score += hand_score(state, i);
        printf("Player #%d score: %d\n", i, p->_score);
    }
}
