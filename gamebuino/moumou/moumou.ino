#include <EEPROM.h>
#include <Gamebuino.h>
#include <SPI.h>

#include "core.h"
#include "ui.h"

#define MAX_CARDS_DRAWN_IN_PILE 10
#define EEPROM_MAGIC_NUMBER 171

Gamebuino gb;
GameState gameState;
UI ui;

void setup() {
    gb.begin();

    GameState *state = &gameState;

    // Initialize positions of piles.
    gameState._deck.x = 1;
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

    gameState._players[0]._hand.x = 4;
    gameState._players[0]._hand.y = 33;
    gameState._players[0]._hand.maxVisibleCards = 7;

    gameState._players[1]._hand.x = 73;
    gameState._players[1]._hand.y = 1;
    gameState._players[1]._hand.maxVisibleCards = 1;
    gameState._players[1]._hand.faceUp = false;

    gameState._players[0]._score = 0;
    gameState._players[1]._score = 0;

    ui.showTitle();
}

void loop() {
    // Main loop.
    if (gb.update()) {
        // Exit to title whenever C is pressed.
        if (gb.buttons.pressed(BTN_C)) {
            ui.pause();
            return;
        }

        // Handle key presses for various modes.
        switch (ui._mode) {
        case selecting:
            handleSelectingButtons();
            break;
        }

        // Draw the board.
        if (ui._mode != gameOver) {
            ui.drawBoard();
            // debug(_cardAnimationCount, _dealingCount);
            ui.debug(gameState._last_card.getSuit(),
                     gameState._last_card.getValue() + 6);
        }

        // Draw other things based on the current state of the game.
        switch (ui._mode) {
        case initialDealing:
        case moving:
            ui.drawDealing();
            break;
        case selecting:
            ui.drawCursor();
            break;
        case gameOver:
            ui.drawWonGame();
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
        if (gameState._deck._count != 0) {
            // drawAndFlip(&gameState._deck, &gameState._players[0]._hand);
            ui._cardAnimationCount = 0;
            ui.animateMove(&gameState._deck, 0, &gameState._players[1]._hand,
                           gameState._players[1]._hand._count);
            ui._dealingCount = ui._cardAnimationCount;
            ui._cardAnimationCount = 0;
            ui._mode = moving;
            // playSoundA();
        }
    } else if (gb.buttons.pressed(BTN_A)) {

        switch (ui._activeLocation) {
        case stock:
            if (gameState._valid_moves._draw) {
                if (gameState._deck._count != 0) {
                    ui._cardAnimationCount = 0;
                    for (int i = 0; i < 1; i++)
                        ui.animateMove(&gameState._deck, 0,
                                       &gameState._players[0]._hand,
                                       gameState._players[0]._hand._count);
                    ui._dealingCount = ui._cardAnimationCount;
                    ui._cardAnimationCount = 0;
                    ui._mode = moving;
                    gameState._input_cmd = CMD_DRAW;
                    ui.playSoundA();
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
                        ui._cardAnimationCount = 0;
                        ui.animateMove(&p, idx, &gameState._played,
                                       gameState._played._count);
                        ui._dealingCount = ui._cardAnimationCount;
                        ui._cardAnimationCount = 0;
                        if (p._count - p.scrollOffset >= p.maxVisibleCards) {
                        } else if (p.scrollOffset > 0) {
                            p.scrollOffset--;
                        } else if (ui._cardIndex == p._count &&
                                   ui._cardIndex > 0) {
                            ui._cardIndex--;
                        }
                        ui._mode = moving;
                        gameState._input_cmd = idx;
                        ui.playSoundA();
                    }
                }
            }
        } break;
        case played:
            if (gameState._valid_moves._pass) {
                ui._cardAnimationCount = 0;
                byte count = gameState._played._count;
                for (int i = 0; i < count; i++)
                    ui.animateMove(&gameState._played, 0, &gameState._table,
                                   gameState._table._count + i);
                ui._dealingCount = ui._cardAnimationCount;
                ui._cardAnimationCount = 0;
                ui._mode = moving;
                gameState._input_cmd = CMD_PASS;
                ui.playSoundA();
            }
            break;
        }
    }
    if (originalLocation != ui._activeLocation)
        ui._cardIndex = 0;
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
            EEPROM.put(address + i + 1, pile->getCard(pile->_count - i - 1));
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
