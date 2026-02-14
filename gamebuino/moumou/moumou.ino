#include <EEPROM.h>
#include <Gamebuino.h>
#include <SPI.h>

#include "ai.h"
#include "core.h"
#include "ui.h"

#define EEPROM_MAGIC_NUMBER 171

Gamebuino gb;
GameState gameState;
UI ui;

void setup() {
    gb.begin();

    GameState *state = &gameState;

    // Initialize positions of piles.
    gameState._deck.x = 2;
    gameState._deck.y = 1;
    gameState._deck.maxVisibleCards = 1;
    gameState._deck.faceUp = false;

    gameState._table.x = 16;
    gameState._table.y = 1;
    gameState._table.maxVisibleCards = 4;
    gameState._table.scrollToLast = true;

    gameState._played.x = 2;
    gameState._played.y = 17;
    gameState._played.maxVisibleCards = 6;
    gameState._played.scrollToLast = true;

    gameState._players[0]._score = 0;
    gameState._players[1]._score = 0;

    ui.showTitle();
}

void loop() {
    // Main loop.
    if (gb.update()) {
        if (ui._mode != MODE_ANIMATE && gameState._pending_cmd != CMD_NONE) {
            process_command(&gameState, &ui);
        }

        // Exit to title whenever C is pressed.
        if (gb.buttons.pressed(BTN_C)) {
            ui.pause();
            return;
        }

        // Handle key presses for various modes.
        switch (ui._mode) {
        case MODE_PLAYER_MOVE:
            handleSelectingButtons();
            break;
        case MODE_SELECT_SUIT:
            handleSuitSelector();
            break;
        case MODE_ROUND_OVER:
            handleRoundOver();
            break;
        }

        // Draw the board.
        if (ui._mode == MODE_ROUND_OVER)
            ui.drawRoundOver(gameState._valid_moves._flags & FLAG_MOUMOU);
        else
            ui.drawBoard();

        // Draw other things based on the current state of the game.
        switch (ui._mode) {
        case MODE_ANIMATE:
            ui.drawDealing();
            break;
        case MODE_PLAYER_MOVE:
            if (gameState._players[gameState._cur_player]._level == Human)
                ui.drawCursor();
            break;
        }
    }
}

void handleSelectingButtons() {
    // Handle buttons when user is using the arrow cursor to navigate.
    Location originalLocation = ui._activeLocation;
    if (gb.buttons.pressed(BTN_RIGHT)) {
        if (ui._activeLocation == hand) {
            if (ui._cardIndex + gameState._players[0]._hand.scrollOffset <
                gameState._players[0]._hand._count - 1) {
                if (ui._cardIndex <
                    gameState._players[0]._hand.maxVisibleCards - 1)
                    ++ui._cardIndex;
                else
                    ++gameState._players[0]._hand.scrollOffset;
            }
        }
    }
    if (gb.buttons.pressed(BTN_LEFT)) {
        if (ui._activeLocation == hand) {
            if (ui._cardIndex > 0)
                --ui._cardIndex;
            else if (gameState._players[0]._hand.scrollOffset > 0)
                --gameState._players[0]._hand.scrollOffset;
        }
    }
    if (gb.buttons.pressed(BTN_DOWN)) {
        if (ui._activeLocation < played)
            ui._activeLocation = played;
        else if (ui._activeLocation = played)
            ui._activeLocation = hand;
    }
    if (gb.buttons.pressed(BTN_UP)) {
        if (ui._activeLocation > stock)
            ui._activeLocation = ui._activeLocation - 1;
    }
    if (gb.buttons.pressed(BTN_B)) {
        /*if (gameState._deck._count != 0) {
            ui._cardAnimationCount = 0;
            ui.animateMove(&gameState._deck, 0, &gameState._players[1]._hand,
                           gameState._players[1]._hand._count);
            ui._dealingCount = ui._cardAnimationCount;
            ui._cardAnimationCount = 0;
            ui._mode = MODE_ANIMATE;
            // playSoundA();
        }*/
    } else if (gb.buttons.pressed(BTN_A)) {

        switch (ui._activeLocation) {
        case stock:
            if (gameState._valid_moves._flags & FLAG_DRAW) {
                if (gameState._deck._count != 0) {
                    gameState._pending_cmd = CMD_DRAW;
                } else {
                    /*while (talonDeck._count != 0) {
                        drawAndFlip(&talonDeck, &gameState._deck);
                    }
                    UndoAction action;
                    action.setFlippedTalon();
                    undo.pushAction(action);*/
                }
            }
            break;
        case hand: {
            Pile &p = gameState._players[0]._hand;
            if (p._count) {
                uint8_t idx = ui._cardIndex + p.scrollOffset;
                for (uint8_t i = 0; i < gameState._valid_moves._count; ++i) {
                    if (idx == gameState._valid_moves._items[i]) {
                        gameState._pending_cmd = idx;
                        break;
                    }
                }
            }
        } break;
        case played:
            if (gameState._valid_moves._flags & FLAG_PASS)
                gameState._pending_cmd = CMD_PASS;
            break;
        }
    }
    if (originalLocation != ui._activeLocation)
        ui._cardIndex = 0;
}

void handleSuitSelector() {
    // Handle buttons when user is using the arrow cursor to navigate.
    if (gb.buttons.pressed(BTN_RIGHT)) {
        if (ui._selected_suit < Clubs)
            ui._selected_suit = ui._selected_suit + 1;
    }
    if (gb.buttons.pressed(BTN_LEFT)) {
        if (ui._selected_suit > Spades)
            ui._selected_suit = ui._selected_suit - 1;
    }
    if (gb.buttons.pressed(BTN_A))
        gameState._pending_cmd = CMD_DEMAND_SPADES + ui._selected_suit;
}

void handleRoundOver() {
    if (gb.buttons.pressed(BTN_A))
        update_score(&gameState);
}

void checkWonGame() {
    /*
    // Check to see if all hands are full
    if (hands[0]._count == 13 &&
        hands[1]._count == 13 &&
        hands[2]._count == 13 &&
        hands[3]._count == 13) {
        ui._mode = gameOver;
        if (cardsToDraw == 1) {
            easyGamesWon++;
            writeEeprom(false);
        } else {
            hardGamesWon++;
            writeEeprom(false);
        }
    }
        */
}
/*
void readEeprom() {
    if (EEPROM.read(0) != EEPROM_MAGIC_NUMBER)
        return;

    EEPROM.get(1, easyGameCount);
    EEPROM.get(3, easyGamesWon);
    EEPROM.get(5, hardGameCount);
    EEPROM.get(7, hardGamesWon);

    // Check to see if saved game.
    if (EEPROM.read(9)) {
        continueGame = true;
        EEPROM.get(10, botLevel);
        int address = 11;
        address += loadPile(address, &gameState._deck);
        address += loadPile(address, &talonDeck);
        for (int i = 0; i < 4; i++)
            address += loadPile(address, &hands[i]);
        for (int i = 0; i < 7; i++)
            address += loadPile(address, &tableau[i]);
    }
    else {
        continueGame = false;
    }
}

void writeEeprom(bool saveGame) {
    EEPROM.update(0, EEPROM_MAGIC_NUMBER);
    EEPROM.put(1, easyGameCount);
    EEPROM.put(3, easyGamesWon);
    EEPROM.put(5, hardGameCount);
    EEPROM.put(7, hardGamesWon);

    EEPROM.update(9, saveGame);
    if (saveGame) {
        EEPROM.put(10, botLevel);
        int address = 11;
        address += savePile(address, &gameState._deck);
        address += savePile(address, &talonDeck);
        for (int i = 0; i < 4; i++)
            address += savePile(address, &hands[i]);
        for (int i = 0; i < 7; i++)
            address += savePile(address, &tableau[i]);
    }
}

int savePile(int address, Pile *pile) {
    EEPROM.put(address, pile->_count);
    for (int i = 0; i < pile->getMaxCards(); i++) {
        if (pile->_count > i) {
            EEPROM.put(address + i + 1, pile->getCard(pile->_count - i -
1));
        }
    }
    return 1 + pile->getMaxCards();
}

int loadPile(int address, Pile *pile) {
    pile->empty();
    byte count = EEPROM.read(address);
    for (byte i = 0; i < count; i++) {
        Card card;
        EEPROM.get(address + i + 1, card);
        pile->addCard(card);
    }
    return 1 + pile->getMaxCards();
}
*/
