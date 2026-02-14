#include "core.h"
#include "ai.h"
#include "ui.h"

void new_round(GameState *state, UI *ui) {
    ui->_activeLocation = stock;
    ui->_cardIndex = 0;
    ui->_cursorX = 0;
    ui->_cursorY = 4;

    state->_players[0]._hand.x = 4;
    state->_players[0]._hand.y = 33;
    state->_players[0]._hand.maxVisibleCards = 7;
    state->_players[0]._hand.faceUp = true;

    state->_players[1]._hand.x = 73;
    state->_players[1]._hand.y = 1;
    state->_players[1]._hand.maxVisibleCards = 1;
    state->_players[1]._hand.faceUp = false;

    state->_deck.newDeck();
    state->_deck.shuffle();

    state->_players[0]._hand.empty();
    state->_players[1]._hand.empty();
    state->_cur_player = 1;
    state->_turn = 0;
    state->_demanded = Undefined;
    state->_valid_moves._flags = FLAG_DRAW;
    state->_last_card = Card();
    state->_moumou_counter = 1;
    state->_pending_cmd = CMD_NEXT_PLAYER;
    state->_fvm_calls = 0;

    state->_played.empty();
    state->_table.empty();

    // Initialize the data structure to deal out the initial board.
    ui->_cardAnimationCount = 0;
    for (int i = 0; i < INITIAL_HAND; i++)
        for (int p = 0; p < 2; p++)
            ui->animateMove(&state->_deck, 0, &state->_players[p]._hand, i);

    state->_last_card = state->_deck._items[0];
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

    if (state->_valid_moves._flags & FLAG_MOUMOU) {
        state->_valid_moves._flags &= ~FLAG_DRAW;
        state->_valid_moves._flags |= FLAG_PASS;
    } else {
        for (uint8_t i = 0; i < p->_hand._count; i++) {
            Card *p_card = &p->_hand._items[i];
            bool allowed = true;
            if ((state->_valid_moves._flags & FLAG_RESTRICT_VALUE) &&
                CardValue(*p_card) != CardValue(state->_last_card)) {
                allowed = false;
            } else if (CardValue(*p_card) != Jack &&
                       CardValue(*p_card) != Six) {
                if (CardValue(*p_card) != CardValue(state->_last_card)) {
                    if (state->_demanded != Undefined)
                        allowed = CardSuit(*p_card) == state->_demanded;
                    else
                        allowed =
                            CardSuit(*p_card) == CardSuit(state->_last_card);
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
    }
    return true;
}

void process_command(GameState *state, UI *ui) {
    state->_input_cmd = state->_pending_cmd;
    state->_pending_cmd = CMD_NONE;
    switch (state->_input_cmd) {
    case CMD_PASS:
        if (state->_players[state->_cur_player]._hand._count == 0 ||
            (state->_valid_moves._flags & FLAG_MOUMOU)) {
            state->_pending_cmd = CMD_ROUND_OVER;
        } else {
            state->_pending_cmd = CMD_NEXT_PLAYER;
            if (state->_played._count && CardValue(state->_last_card) == Jack) {
                state->_pending_cmd = ai_demand_suit(state);
            }
            state->_valid_moves._flags &= ~FLAG_PASS;
            state->_valid_moves._flags &= ~FLAG_RESTRICT_VALUE;
        }
        ui->startPass();
        break;
    case CMD_DRAW:
        state->_pending_cmd = CMD_SELECT_MOVE;
        state->_valid_moves._flags &= ~FLAG_DRAW;
        ui->startDraw();
        break;
    case CMD_NEXT_PLAYER:
        state->_cur_player = (state->_cur_player + 1) % PLAYER_COUNT;
        state->_pending_cmd = CMD_SELECT_MOVE;
        state->_valid_moves._flags |= FLAG_DRAW;
        break;
    case CMD_SELECT_MOVE:
        find_valid_moves(state, state->_cur_player);
        state->_pending_cmd = ai_move(state);
        break;
    case CMD_SELECT_SUIT:
        break;
    case CMD_ROUND_OVER:
        state->_players[0]._hand.x = 4;
        state->_players[0]._hand.y = 33;
        state->_players[0]._hand.maxVisibleCards = 6;
        state->_players[0]._hand.setFace(true);

        state->_players[1]._hand.x = 4;
        state->_players[1]._hand.y = 1;
        state->_players[1]._hand.maxVisibleCards = 6;
        state->_players[1]._hand.setFace(true);
        ui->_mode = MODE_ROUND_OVER;
        ui->_drawRoundOverTimer = ROUND_OVER_TIMER;
        break;
    case CMD_NEW_ROUND:
        new_round(state, ui);
        break;
    case CMD_DEMAND_SPADES:
    case CMD_DEMAND_HEARTS:
    case CMD_DEMAND_DIAMOND:
    case CMD_DEMAND_CLUBS:
        state->_demanded =
            static_cast<uint8_t>(state->_input_cmd) - CMD_DEMAND_SPADES;
        state->_pending_cmd = CMD_NEXT_PLAYER;
        break;
    default: {
        Player *p = &state->_players[state->_cur_player];
        Card p_card = p->_hand._items[state->_input_cmd];
        ui->startPlayCard(state->_input_cmd);
        // apply effects
        state->_valid_moves._flags &= ~FLAG_OPPONENT_SKIPS;
        if (CardValue(p_card) == Eight) {
            state->_valid_moves._flags |= FLAG_OPPONENT_SKIPS;
        }
        if (CardValue(p_card) == Ace) {
            state->_valid_moves._flags |= FLAG_OPPONENT_SKIPS;
        }
        if (CardSuit(p_card) == Clubs && CardValue(p_card) == King) {
            state->_valid_moves._flags |= FLAG_OPPONENT_SKIPS;
        }
        // check moumou
        if (CardValue(p_card) == CardValue(state->_last_card)) {
            ++state->_moumou_counter;
            if (state->_moumou_counter == SuitCount)
                state->_valid_moves._flags |= FLAG_MOUMOU;
        } else
            state->_moumou_counter = 1;

        if (CardValue(p_card) == Six ||
            (state->_valid_moves._flags & FLAG_OPPONENT_SKIPS))
            state->_valid_moves._flags |= FLAG_DRAW;
        else
            state->_valid_moves._flags &= ~FLAG_DRAW;
        if (CardValue(p_card) != Six)
            state->_valid_moves._flags |= FLAG_PASS;
        else
            state->_valid_moves._flags &= ~FLAG_PASS;

        state->_last_card = p_card;

        state->_demanded = Undefined;

        if (!(state->_valid_moves._flags & FLAG_OPPONENT_SKIPS) &&
            CardValue(state->_last_card) != Six)
            state->_valid_moves._flags |= FLAG_RESTRICT_VALUE;
        else
            state->_valid_moves._flags &= ~FLAG_RESTRICT_VALUE;

        if (CardValue(state->_last_card) != Six)
            state->_valid_moves._flags |= FLAG_PASS;
        else
            state->_valid_moves._flags &= ~FLAG_PASS;

        state->_pending_cmd = CMD_SELECT_MOVE;
    } break;
    }
}

void update_score(GameState *state) {
    state->_players[0]._score += hand_score(state, 0);
    state->_players[1]._score += hand_score(state, 1);
    state->_pending_cmd = CMD_NEW_ROUND;
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

void check_recycle_deck(GameState *state, uint8_t *cards_to_draw) {
    if (state->_deck._count <= cards_to_draw) {
        if (state->_table._count < 4) {
            *cards_to_draw = max(0, state->_deck._count);
        } else {
            state->_table.removeCards(state->_table._count - 4, &state->_deck);
            state->_deck.shuffle();
        }
    }
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
