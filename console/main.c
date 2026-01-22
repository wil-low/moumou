#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const uint8_t Undefined = ' ';

typedef enum : uint8_t
{
    Spades = 's',
    Hearts = 'h',
    Diamonds = 'd',
    Clubs = 'c'
} Suit;

typedef enum : uint8_t
{
    Six = '6',
    Seven = '7',
    Eight = '8',
    Nine = '9',
    Ten = 'T',
    Jack = 'J',
    Queen = 'Q',
    King = 'K',
    Ace = 'A'
} Value;

const Suit suits[] = {Spades, Hearts, Diamonds, Clubs};
const Value values[] = {Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace};

typedef struct
{
    Value _value;
    Suit _suit;
} Card;

typedef struct
{
    uint8_t _count;
    Card _items[9 * 4];
} Deck;

typedef enum : uint8_t
{
    Human = 1 << 0,
    Dealt = 1 << 1,
} Flag;

#define MAX_CARDS_IN_HAND 16
#define CMD_DRAW 253
#define CMD_PASS 254
#define CMD_REPEAT_FIND 255

#define PLAY_OK 0
#define PLAY_DEMAND_SUIT 1
#define PLAY_OPPONENT_SKIPS 2
#define PLAY_MOUMOU 3

#define PLAYER_COUNT 2
#define INITIAL_HAND 2

typedef struct
{
    uint8_t _flags;
    Deck _hand;
    uint8_t _score;
} Player;

typedef struct
{
    uint8_t _items[MAX_CARDS_IN_HAND];
    uint8_t _count;
    bool _draw;
    bool _pass;
} ValidMoves;

typedef struct
{
    Player _players[PLAYER_COUNT];
    Deck _deck;
    Deck _table;
    Deck _played;
    uint8_t _cur_player;
    uint8_t _turn;
    Suit _demanded;
    ValidMoves _valid_moves;
    Card _last_card;
    uint8_t _moumou_counter;
} GameState;

GameState game_state;

void recycle_deck(GameState *state)
{
    state->_deck._count = state->_table._count - 3;
    for (uint8_t i = 0; i < state->_deck._count; ++i)
    {
        state->_deck._items[i] = state->_table._items[i];
    }
    for (uint8_t i = 0; i < 3; ++i)
    {
        state->_table._items[i] = state->_table._items[state->_deck._count + i];
    }
    state->_table._count = 3;
}

void move_played_to_table(GameState *state)
{
    for (uint8_t i = 0; i < state->_played._count; ++i)
    {
        state->_table._items[state->_table._count++] = state->_played._items[i];
    }
    state->_played._count = 0;
    state->_valid_moves._draw = true;
}

Card deal(GameState *state)
{
    if (state->_deck._count > 0)
    {
        int idx = rand() % state->_deck._count;
        state->_deck._count--;
        Card tmp = state->_deck._items[idx];
        state->_deck._items[idx] = state->_deck._items[state->_deck._count];
        return tmp;
    }
    recycle_deck(state);
    return deal(state);
}

bool draw(GameState *state, uint8_t player_idx, uint8_t count)
{
    Player *p = &state->_players[player_idx];
    for (uint8_t i = 0; i < count; i++)
    {
        if (p->_hand._count < MAX_CARDS_IN_HAND)
        {
            Card card = deal(state);
            p->_hand._items[p->_hand._count] = card;
            p->_hand._count++;
        }
    }
    return true;
}

uint8_t play_card(GameState *state, uint8_t player_idx, uint8_t card_idx)
{
    Player *p = &state->_players[player_idx];
    Card *p_card = &p->_hand._items[card_idx];
    // apply effects
    uint8_t result = PLAY_OK;
    uint8_t next_player = (player_idx + 1) % PLAYER_COUNT;
    if (p_card->_value == Eight)
    {
        draw(state, next_player, 2);
        result = PLAY_OPPONENT_SKIPS;
    }
    if (p_card->_value == Ace)
    {
        result = PLAY_OPPONENT_SKIPS;
    }
    if (p_card->_value == Seven)
    {
        draw(state, next_player, 1);
    }
    if (p_card->_suit == Clubs && p_card->_value == King)
    {
        draw(state, next_player, 5);
        result = PLAY_OPPONENT_SKIPS;
    }
    if (p_card->_value == Jack)
        result = PLAY_DEMAND_SUIT;

    // check moumou
    if (p_card->_value == state->_last_card._value)
    {
        ++state->_moumou_counter;
        if (state->_moumou_counter == 4)
            result = PLAY_MOUMOU;
    }
    else
        state->_moumou_counter = 1;

    state->_valid_moves._draw =
        p_card->_value == Six || result == PLAY_OPPONENT_SKIPS;
    state->_valid_moves._pass = p_card->_value != Six;

    state->_last_card = *p_card;
    state->_played._items[state->_played._count++] = *p_card;
    for (uint8_t i = card_idx + 1; i < p->_hand._count; ++i)
    {
        p->_hand._items[i - 1] = p->_hand._items[i];
    }
    p->_hand._count--;

    return result;
}

bool find_valid_moves(GameState *state, uint8_t player_idx)
{
    Player *p = &state->_players[player_idx];
    if (p->_hand._count == 0)
        return false;

    state->_valid_moves._count = 0;

    for (uint8_t i = 0; i < p->_hand._count; i++)
    {
        Card *p_card = &p->_hand._items[i];
        if (state->_played._count)
        {
            if (state->_last_card._value == Six)
            {
                if (p_card->_value != Jack &&
                    p_card->_value != state->_last_card._value &&
                    p_card->_suit != state->_last_card._suit)
                    continue;
            }
            else if (p_card->_value != state->_last_card._value)
                continue;
        }
        else
        {
            if (p_card->_value == Jack || p_card->_value == Six)
            {
            }
            else if (state->_demanded != Undefined)
            {
                if (p_card->_suit != state->_demanded)
                    continue;
            }
            else if (state->_last_card._value != Undefined &&
                     (p_card->_value == state->_last_card._value ||
                      p_card->_suit == state->_last_card._suit))
            {
            }
            else
            {
                continue;
            }
        }
        state->_valid_moves._items[state->_valid_moves._count++] = i;
    }
    if (state->_last_card._value == Six)
    {
        state->_valid_moves._draw = true;
        state->_valid_moves._pass = false;
    }
    if (state->_valid_moves._count == 0 && !state->_valid_moves._draw)
        state->_valid_moves._pass = true;
    return true;
}

uint8_t input_move(GameState *state)
{
    int result;
    while (true)
    {
        printf("Enter your choice: ");
        char cmd[5];
        scanf("%s", cmd);
        if (strcmp(cmd, "d") == 0 && state->_valid_moves._draw)
            return CMD_DRAW;
        if (strcmp(cmd, "p") == 0 && state->_valid_moves._pass)
            return CMD_PASS;
        if (strcmp(cmd, "r") == 0)
            return CMD_REPEAT_FIND;
        if (sscanf(cmd, "%d", &result) == 1)
        {
            if (result >= 0 && result < state->_valid_moves._count)
                return result;
        }
    }
    return 0; // never reach here
}

Suit input_suit(GameState *)
{
    while (true)
    {
        printf("Select suit (shdc): ");
        char suit[5];
        scanf("%s", suit);
        if (suit[0] == Spades || suit[0] == Hearts || suit[0] == Diamonds ||
            suit[0] == Clubs)
        {
            return suit[0];
        }
    }
    return Undefined;
}

void input_wait(const char *message)
{
    printf("%s", message);
    /* flush pending input */
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
    /* wait for Enter */
    getchar();
}

void render_card(Card *card)
{
    printf("%c%c ", card->_value, card->_suit);
}

void render_player(GameState *state, uint8_t idx)
{
    Player *p = &state->_players[idx];
    printf("Player #%d (%s), cards %d, score %d\n", idx,
           (p->_flags & Human) ? "human" : "robot", p->_hand._count, p->_score);
    printf("    ");
    for (uint8_t i = 0; i < p->_hand._count; ++i)
    {
        render_card(&p->_hand._items[i]);
    }
    printf("  Draw   Pass");
    if (idx == state->_cur_player)
    {
        // show valid moves
        printf("\n");
        printf("    ");
        uint8_t idx = 0;
        for (uint8_t i = 0; i < p->_hand._count; ++i)
        {
            if (idx < state->_valid_moves._count &&
                i == state->_valid_moves._items[idx])
            {
                printf("%2d ", idx);
                idx++;
            }
            else
                printf("   ");
        }
        if (state->_valid_moves._draw)
            printf("     d");
        else
            printf("      ");
        if (state->_valid_moves._pass)
            printf("      p");
    }
    else
    {
        printf("\n");
    }
    printf("\n\n");
}

void render_table(GameState *state)
{
    printf("Table");
    if (state->_demanded != Undefined)
        printf(", demanded: %c", state->_demanded);
    printf("\n  > ");
    uint8_t start = state->_table._count > 4 ? state->_table._count - 4 : 0;
    for (uint8_t i = start; i < state->_table._count; ++i)
    {
        render_card(&state->_table._items[i]);
    }
    printf("\nPlayed\n  > ");
    for (uint8_t i = 0; i < state->_played._count; ++i)
    {
        render_card(&state->_played._items[i]);
    }
    printf("\n\n");
}

void render_game(GameState *state)
{
    printf("\n======================== Turn %d ========================\n\n",
           state->_turn);
    render_player(state, 1 - state->_cur_player);
    render_table(state);
    render_player(state, state->_cur_player);
}

void new_round(GameState *state)
{
    for (uint8_t i = 0; i < PLAYER_COUNT; ++i)
    {
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
    state->_deck._count = 9 * 4;
    state->_table._count = 0;
    state->_played._count = 0;
    uint8_t idx = 0;
    for (uint8_t s = 0; s < sizeof(suits); ++s)
    {
        for (uint8_t v = 0; v < sizeof(values); ++v)
        {
            state->_deck._items[idx]._value = values[v];
            state->_deck._items[idx]._suit = suits[s];
            ++idx;
        }
    }

    // Deal initial cards
    draw(state, 0, INITIAL_HAND);
    draw(state, 1, INITIAL_HAND);
    play_card(state, 0, 0);
}

void calculate_scores(GameState *state)
{
    for (uint8_t i = 0; i < PLAYER_COUNT; ++i)
    {
        Player *p = &state->_players[i];
        for (uint8_t idx = 0; idx < p->_hand._count; ++idx)
        {
            Value val = p->_hand._items[idx]._value;
            switch (val)
            {
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

void setup()
{
    printf("Moumou\n");
    srand(time(NULL));

    GameState *state = &game_state;

    for (uint8_t i = 0; i < PLAYER_COUNT; ++i)
    {
        state->_players[i]._flags = (i == 0) ? Human : 0;
        state->_players[i]._score = 0;
    }
    new_round(state);
}

void loop()
{
    GameState *state = &game_state;
    bool has_cards = find_valid_moves(state, state->_cur_player);
    render_game(state);

    if (!has_cards)
    {
        printf("Player #%d has no cards\n", state->_cur_player);
        calculate_scores(state);
        new_round(state);
        return;
    }

    uint8_t card_idx = input_move(state);

    if (card_idx == CMD_DRAW)
    {
        draw(state, state->_cur_player, 1);
        state->_valid_moves._draw = false;
    }
    else if (card_idx == CMD_PASS)
    {
    }
    else if (card_idx == CMD_REPEAT_FIND)
    {
    }
    else
    {
        card_idx = state->_valid_moves._items[card_idx];
        uint8_t result = play_card(state, state->_cur_player, card_idx);
        state->_demanded = Undefined;
        if (result == PLAY_DEMAND_SUIT)
        {
            state->_demanded = input_suit(0);
        }
        else if (result == PLAY_OPPONENT_SKIPS)
        {
            move_played_to_table(state);
        }
        else if (result == PLAY_MOUMOU)
        {
            printf("Player #%d declares Moumou\n", state->_cur_player);
            calculate_scores(state);
            new_round(state);
            return;
        }
    }
    state->_valid_moves._pass = state->_last_card._value != Six;

    if (card_idx == CMD_PASS)
    {
        move_played_to_table(state);
        state->_valid_moves._pass = false;
        state->_turn++;
        state->_cur_player = (state->_cur_player + 1) % PLAYER_COUNT;
    }
}

int main(/*int argc, char **argv*/)
{
    setup();

    while (true)
        loop();

    return 0;
}
