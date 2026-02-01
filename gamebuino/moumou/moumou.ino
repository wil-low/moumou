#include "pile.h"
#include "undo.h"
#include <EEPROM.h>
#include <Gamebuino.h>
#include <SPI.h>

#define MAX_CARDS_DRAWN_IN_PILE 10
#define EEPROM_MAGIC_NUMBER 171
#define INITIAL_HAND 5

#define PSEUDO_GRAY GRAY
// #define PSEUDO_GRAY BLACK

Gamebuino gb;

enum GameMode {
    initialDealing,
    selecting,
    moving,
    resolving,
    passing,
    gameOver
};
// State of the game.
GameMode mode = selecting;

// how many cards must be animated in dealing mode
byte dealingCount;

// Stock: where you draw cards from
// Table: table cards
// Played: cards played by human
// Human: cards in human hand
enum Location {
    table,
    stock,
    played,
    hand
};
// Stack that the cursor is currently pointed at.
Location activeLocation;
// Within the human hand, card position on the screen, 0 being left card.
byte cardIndex;
// Position of the cursor for animation.
byte cursorX, cursorY;

// AI level
byte botLevel;

// Keep track of source pile for returning invalid moves.
Pile *sourcePile;

Pile stockDeck(36, 1);
Pile tableRow(36, 4);
Pile playedRow(10, 6);
Pile playerDeck[2] = {Pile(30, 7), Pile(30, 1)};
uint16_t playerScore[2];

byte curPlayer;

UndoStack undo;

struct CardAnimation {
    Card card;
    byte x, y, destX, destY;
    Pile *destination;
};

// Used to deal at the start of the game.
CardAnimation cardAnimations[20];
byte cardAnimationCount = 0;

struct CardBounce {
    Card card;
    int x, y, xVelocity, yVelocity;
};

// Keeps track of a bouncing card for the winning animation.
CardBounce bounce;
byte bounceIndex;

int easyGameCount, easyGamesWon, hardGameCount, hardGamesWon;

const char easyOption[] PROGMEM = "Bot level 1";
const char hardOption[] PROGMEM = "Bot level 2";
const char statisticsOption[] PROGMEM = "Game statistics";
const char *const newGameMenu[3] PROGMEM = {easyOption, hardOption,
                                            statisticsOption};

const char quitOption[] PROGMEM = "Quit game";
const char resumeOption[] PROGMEM = "Resume game";
const char saveOption[] PROGMEM = "Save for later";
const char undoOption[] PROGMEM = "Undo last move";
const char *const pauseMenu[5] PROGMEM = {
    resumeOption, quitOption, statisticsOption, saveOption, undoOption};

bool continueGame;

const byte title[] PROGMEM = {
    64,   36,   0x0,  0x0,  0x80, 0x0,  0x1,  0x24, 0x0,  0x0,  0x0,  0x84,
    0x80, 0x0,  0x1,  0x4,  0x0,  0x0,  0x0,  0x88, 0x80, 0x0,  0x1,  0x4,
    0x0,  0x0,  0x0,  0x90, 0x8F, 0x16, 0x1D, 0x24, 0xCE, 0x0,  0x0,  0xA0,
    0x99, 0x99, 0x33, 0x25, 0x99, 0x0,  0x0,  0xC0, 0x90, 0x91, 0x21, 0x25,
    0x11, 0x0,  0x0,  0xA0, 0x90, 0x91, 0x21, 0x26, 0x1F, 0x0,  0x0,  0x90,
    0x90, 0x91, 0x21, 0x25, 0x10, 0x0,  0x0,  0x88, 0x99, 0x91, 0x33, 0x25,
    0x98, 0x0,  0x0,  0x84, 0x8F, 0x11, 0x1D, 0x24, 0xCF, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x10, 0x1C, 0x70, 0xE,  0x0,  0x10, 0x0,  0x0,  0x10,
    0x3E, 0xF8, 0x3F, 0x80, 0x10, 0x0,  0x0,  0x38, 0x3E, 0xF8, 0x3F, 0x80,
    0x38, 0x0,  0x0,  0x7C, 0x3F, 0xF8, 0x3F, 0x80, 0x7C, 0x0,  0x0,  0xFE,
    0x3F, 0xF9, 0xDF, 0x70, 0xFE, 0x0,  0x1,  0xFF, 0x1F, 0xF3, 0xFF, 0xF9,
    0xFF, 0x0,  0x3,  0xFF, 0x8F, 0xF3, 0xFF, 0xFB, 0xFF, 0x80, 0x3,  0xFF,
    0x8F, 0xE3, 0xFF, 0xF9, 0xFF, 0x0,  0x3,  0xFF, 0x87, 0xC3, 0xF5, 0xF8,
    0xFE, 0x0,  0x1,  0xD7, 0x3,  0x81, 0xE4, 0xF0, 0x7C, 0x0,  0x0,  0x10,
    0x3,  0x80, 0x4,  0x0,  0x38, 0x0,  0x0,  0x38, 0x1,  0x0,  0xE,  0x0,
    0x10, 0x0,  0x0,  0xFE, 0x1,  0x0,  0x3F, 0x80, 0x10, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x90, 0x0,  0x40,
    0x0,  0x0,  0x0,  0x3C, 0x0,  0x82, 0x0,  0x0,  0x0,  0x0,  0x0,  0x44,
    0x0,  0x82, 0x0,  0x0,  0x0,  0x0,  0x0,  0x40, 0x78, 0x97, 0x9C, 0x4B,
    0x38, 0x0,  0x0,  0x60, 0xCC, 0x92, 0x2,  0x4C, 0x64, 0x0,  0x0,  0x18,
    0x84, 0x92, 0x2,  0x48, 0x44, 0x0,  0x0,  0xC,  0x84, 0x92, 0x1E, 0x48,
    0x7C, 0x0,  0x0,  0x4,  0x84, 0x92, 0x22, 0x48, 0x40, 0x0,  0x0,  0x44,
    0xCC, 0x92, 0x22, 0x48, 0x60, 0x0,  0x0,  0x78, 0x78, 0x93, 0x9E, 0x48,
    0x3C, 0x0,
};

void setup() {
    gb.begin();

    // Initialize positions of piles.
    stockDeck.x = 1;
    stockDeck.y = 0;

    tableRow.x = 16;
    tableRow.y = 0;

    playedRow.x = 2;
    playedRow.y = 17;

    playerDeck[0].x = 4;
    playerDeck[0].y = 33;
    playerScore[0] = 0;

    playerDeck[1].x = 73;
    playerDeck[1].y = 0;
    playerScore[1] = 0;

    showTitle();
}

void loop() {
    // Main loop.
    if (gb.update()) {
        // Exit to title whenever C is pressed.
        if (gb.buttons.pressed(BTN_C)) {
            pause();
            return;
        }

        // Handle key presses for various modes.
        switch (mode) {
        case selecting:
            handleSelectingButtons();
            break;
        }

        // Draw the board.
        if (mode != gameOver) {
            drawBoard();
            drawNumberRight(cardAnimationCount, 83, 33);
            drawNumberRight(dealingCount, 83, 41);
        }

        // Draw other things based on the current state of the game.
        switch (mode) {
        case initialDealing:
        case moving:
            drawDealing();
            break;
        case selecting:
            drawCursor();
            break;
        case gameOver:
            drawWonGame();
            break;
        }
    }
}

void showTitle() {
start:
    gb.display.persistence = true;
    gb.titleScreen(F(""), title);
    randomSeed(45);
    // gb.pickRandomSeed();
    gb.battery.show = false;
    setupNewGame();
    readEeprom();

    // If there is a saved game in EEPROM, just skip right to the game.
    if (continueGame) {
        writeEeprom(false);
        mode = selecting;
        return;
    }

    // Ask whether we want easy (flip 1 card per draw) or hard (flip 3 cards per
    // draw).
    char menuOption;
askAgain:
    menuOption = gb.menu(newGameMenu, 3);
    if (menuOption == -1)
        goto start;

    if (menuOption == 0) {
        botLevel = 1;
        easyGameCount++;
        writeEeprom(false);
    } else if (menuOption == 1) {
        botLevel = 2;
        hardGameCount++;
        writeEeprom(false);
    } else {
        displayStatistics();
        goto askAgain;
    }
}

void pause() {
askAgain:
    switch (
        gb.menu(pauseMenu, mode == selecting ? (undo.isEmpty() ? 4 : 5) : 3)) {
    case 2:
        // statistics
        displayStatistics();
        goto askAgain;
    case 1:
        // Quit the game
        showTitle();
        break;
    case 3:
        // Save for later
        writeEeprom(true);
        showTitle();
        break;
    case 4:
        // Undo
        performUndo();
        break;
    case 0:
    default:
        // Resume the game
        break;
    }
}

void setupNewGame() {
    undo = UndoStack();
    activeLocation = stock;
    cardIndex = 0;
    cursorX = 11;
    cursorY = 4;

    stockDeck.newDeck();
    stockDeck.shuffle();

    playerDeck[0].empty();
    playerDeck[1].empty();
    curPlayer = 0;

    playedRow.empty();
    tableRow.empty();

    // Initialize the data structure to deal out the initial board.
    cardAnimationCount = 0;
    for (int i = 0; i < INITIAL_HAND; i++)
        for (int p = 0; p < 2; p++)
            animateMove(&stockDeck, 0, &playerDeck[p], i);
    dealingCount = cardAnimationCount;
    cardAnimationCount = 0;
    mode = initialDealing;
}

void animateMove(Pile *src, byte srcIdx, Pile *dst, byte dstIdx) {
    Card card = src->removeCardAt(srcIdx);
    if (src == &stockDeck && dst == &playerDeck[0])
        card.flip();
    cardAnimations[cardAnimationCount] = CardAnimation();
    cardAnimations[cardAnimationCount].x =
        src->x + (src->getMaxVisibleCards() > 1 ? srcIdx * 11 : 0);
    cardAnimations[cardAnimationCount].y = src->y;
    cardAnimations[cardAnimationCount].destX =
        dst->x + (dst->getMaxVisibleCards() > 1 ? dstIdx * 11 : 0);
    cardAnimations[cardAnimationCount].destY = dst->y;
    cardAnimations[cardAnimationCount].destination = dst;
    cardAnimations[cardAnimationCount].card = card;
    cardAnimationCount++;
}

const uint16_t patternA[] PROGMEM = {0x0045, 0x0118, 0x0000};
const uint16_t patternB[] PROGMEM = {0x0045, 0x0108, 0x0000};

void handleSelectingButtons() {
    // Handle buttons when user is using the arrow cursor to navigate.
    Location originalLocation = activeLocation;
    if (gb.buttons.pressed(BTN_RIGHT)) {
        if (activeLocation == hand) {
            if (cardIndex + playerDeck[0].cardOffset <
                playerDeck[0].getCardCount() - 1) {
                if (cardIndex < playerDeck[0].getMaxVisibleCards() - 1)
                    ++cardIndex;
                else
                    ++playerDeck[0].cardOffset;
            }
        }
    }
    if (gb.buttons.pressed(BTN_LEFT)) {
        if (activeLocation == hand) {
            if (cardIndex > 0)
                --cardIndex;
            else if (playerDeck[0].cardOffset > 0)
                --playerDeck[0].cardOffset;
        }
    }
    if (gb.buttons.pressed(BTN_DOWN)) {
        if (activeLocation < played)
            activeLocation = played;
        else if (activeLocation = played)
            activeLocation = hand;
    }
    if (gb.buttons.pressed(BTN_UP)) {
        if (activeLocation > stock)
            activeLocation = activeLocation - 1;
    }
    if (gb.buttons.pressed(BTN_B)) {
        if (stockDeck.getCardCount() != 0) {
            // drawAndFlip(&stockDeck, &playerDeck[0]);
            cardAnimationCount = 0;
            animateMove(&stockDeck, 0, &playerDeck[1],
                        playerDeck[1].getCardCount());
            dealingCount = cardAnimationCount;
            cardAnimationCount = 0;
            mode = moving;
            // playSoundA();
        }
        /*
        if (activeLocation >= tableau1 || activeLocation == talon) {
            Pile *pile = getActiveLocationPile();
            if (pile->getCardCount() > 0) {
                Card card = pile->getCard(0);
                bool foundMatch = false;
                for (int i = 0; i < 4; i++) {
                    if (hands[i].getCardCount() == 0) {
                        if (card.getValue() == ace) {
                            foundMatch = true;
                        }
                    } else {
                        Card card1 = hands[i].getCard(0);
                        Card card2 = pile->getCard(0);
                        if (card1.getSuit() == card2.getSuit() &&
                            card1.getValue() + 1 == card2.getValue()) {
                            foundMatch = true;
                        }
                    }
                    if (foundMatch) {
                        moving.empty();
                        moving.x = pile->x;
                        moving.y = cardYPosition(pile, 0);
                        moving.addCard(pile->removeTopCard());
                        sourcePile = &hands[i];
                        mode = fastFoundation;
                        playSoundA();
                        break;
                    }
                }
            }
        }*/
    } else if (gb.buttons.pressed(BTN_A)) {

        switch (activeLocation) {
        case stock:
            if (stockDeck.getCardCount() != 0) {
                // drawAndFlip(&stockDeck, &playerDeck[0]);
                cardAnimationCount = 0;
                for (int i = 0; i < 1; i++)
                    animateMove(&stockDeck, 0, &playerDeck[0],
                                playerDeck[0].getCardCount());
                dealingCount = cardAnimationCount;
                cardAnimationCount = 0;
                mode = moving;
                // playSoundA();
            } else {
                /*while (talonDeck.getCardCount() != 0) {
                    drawAndFlip(&talonDeck, &stockDeck);
                }
                UndoAction action;
                action.setFlippedTalon();
                undo.pushAction(action);*/
            }
            break;
        case hand:
            cardAnimationCount = 0;
            animateMove(&playerDeck[0], cardIndex, &playedRow,
                        playedRow.getCardCount());
            dealingCount = cardAnimationCount;
            cardAnimationCount = 0;
            mode = moving;
            break;
        case played:
            cardAnimationCount = 0;
            byte count = playedRow.getCardCount();
            for (int i = 0; i < count; i++)
                animateMove(&playedRow, 0, &tableRow,
                            tableRow.getCardCount() + i);
            dealingCount = cardAnimationCount;
            cardAnimationCount = 0;
            mode = moving;
            break;
        }
    }
    if (originalLocation != activeLocation)
        cardIndex = 0;
}

void handleMovingPileButtons() {
    /*
    // Handle buttons when user is moving a pile of cards.
    if (gb.buttons.pressed(BTN_RIGHT)) {
        if (activeLocation != hand4 && activeLocation != tableau7) {
            activeLocation = activeLocation + 1;
        }
    }
    if (gb.buttons.pressed(BTN_LEFT)) {
        if (activeLocation != talon && activeLocation != tableau1) {
            activeLocation = activeLocation - 1;
        }
    }
    if (gb.buttons.pressed(BTN_DOWN)) {
        if (activeLocation == talon)
            activeLocation = tableau2;
        else if (activeLocation <= hand4)
            activeLocation = activeLocation + 7;
    }
    if (gb.buttons.pressed(BTN_UP)) {
        if (activeLocation >= tableau4)
            activeLocation = activeLocation - 7;
        else if (activeLocation >= tableau1)
            activeLocation = talon;
    }
    if (gb.buttons.pressed(BTN_A)) {
        playSoundB();
        switch (activeLocation) {
        case talon:
            mode = illegalMove;
            break;
        case hand1:
        case hand2:
        case hand3:
        case hand4: {
            if (moving.getCardCount() != 1) {
                mode = illegalMove;
                break;
            }
            Pile *destinationFoundation = getActiveLocationPile();
            if (destinationFoundation->getCardCount() == 0) {
                if (moving.getCard(0).getValue() != ace) {
                    mode = illegalMove;
                    break;
                }
            } else {
                Card card1 = destinationFoundation->getCard(0);
                Card card2 = moving.getCard(0);
                if (card1.getSuit() != card2.getSuit() ||
                    card1.getValue() + 1 != card2.getValue()) {
                    mode = illegalMove;
                    break;
                }
            }
            moveCards();
            checkWonGame();
        } break;
        case tableau1:
        case tableau2:
        case tableau3:
        case tableau4:
        case tableau5:
        case tableau6:
        case tableau7: {
            Pile *destinationTableau = getActiveLocationPile();
            if (destinationTableau->getCardCount() > 0) {
                // Make sure that it is a decending value, alternating
color. Card card1 = destinationTableau->getCard(0); Card card2 =
moving.getCard(moving.getCardCount() - 1); if (card1.isRed() ==
card2.isRed() || card1.getValue() != card2.getValue() + 1) { mode =
illegalMove; break;
                }
            } else {
                // You can only place kings in an empty tableau.
                Card card = moving.getCard(moving.getCardCount() - 1);
                if (card.getValue() != king) {
                    mode = illegalMove;
                    break;
                }
            }
        }
            moveCards();
            break;
        }
    }
        */
}

void moveCards() {
    /*
    Pile *pile = getActiveLocationPile();
    UndoAction action;
    action.source = sourcePile;
    action.destination = pile;
    action.setCardCount(moving.getCardCount());
    pile->addPile(&moving);
    mode = selecting;
    if (updateAfterPlay()) {
        action.setRevealed();
    }
    undo.pushAction(action);
    */
}

bool updateAfterPlay() {
    bool result = revealCards();
    checkWonGame();
    cardIndex = 0;
    bool unused;
    getCursorDestination(cursorX, cursorY, unused);
    return result;
}

bool revealCards() {
    bool revealed = false;
    /*
    // Check for cards to reveal.
    for (int i = 0; i < 7; i++) {
        if (tableau[i].getCardCount() == 0)
            continue;
        Card card = tableau[i].removeTopCard();
        if (card.isFaceDown()) {
            card.flip();
            revealed = true;
        }
        tableau[i].addCard(card);
    }
        */
    return revealed;
}

void checkWonGame() {
    /*
    // Check to see if all hands are full
    if (hands[0].getCardCount() == 13 &&
        hands[1].getCardCount() == 13 &&
        hands[2].getCardCount() == 13 &&
        hands[3].getCardCount() == 13) {
        mode = gameOver;
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
void drawDeck(Pile *deck) {
    for (int i = 0; i < min(deck->getMaxVisibleCards(),
deck->getCardCount()); i++) { drawCard( deck->x + i * 11, deck->y,
            deck->getCard(
                min(deck->getMaxVisibleCards(), deck->getCardCount()) - i -
1));
    }
}
*/

void drawDeck(Pile *deck) {
    for (int i = 0; i < deck->getMaxVisibleCards(); ++i) {
        if (i + deck->cardOffset >= deck->getCardCount())
            break;
        drawCard(
            deck->x + i * 11, deck->y,
            deck->getCard(deck->getCardCount() - (i + deck->cardOffset) - 1));
    }
}

void drawNumberRight(uint16_t n, byte x, byte y) {
    byte cur_x = x - 3;
    if (n == 0) {
        gb.display.drawChar(cur_x, y, '0', 1);
        return;
    }
    while (n > 0) {
        gb.display.drawChar(cur_x, y, (n % 10) + '0', 1);
        n = n / 10;
        cur_x -= 4;
    }
}

void drawBoard() {
    // Stock
    if (stockDeck.getCardCount() != 0) {
        drawCard(stockDeck.x, stockDeck.y, Card(undef, spade, true));
    }

    // Scores
    drawNumberRight(playedRow.getCardCount(), 83, 17);
    // drawNumberRight(playerScore[1], 83, 17);
    drawNumberRight(playerScore[0], 83, 25);

    // Bot deck
    if (playerDeck[1].getCardCount() != 0) {
        drawCard(playerDeck[1].x, playerDeck[1].y, Card(undef, spade, true));
        gb.display.setColor(WHITE);
        drawNumberRight(playerDeck[1].getCardCount(), 81, 2);
    }

    // Human deck
    drawDeck(&playerDeck[0]);
    if (playerDeck[0].cardOffset > 0)
        drawLeftArrow(0, 38);
    if (playerDeck[0].getCardCount() - playerDeck[0].cardOffset >
        playerDeck[0].getMaxVisibleCards())
        drawRightArrow(81, 38);

    drawDeck(&playedRow);
    drawDeck(&tableRow);
    /*
        // Talon
        for (int i = 0; i < min(3, talonDeck.getCardCount()); i++) {
            drawCard(talonDeck.x + i * 2, talonDeck.y,
                     talonDeck.getCard(min(3, talonDeck.getCardCount())
       - i
       - 1));
        }

        // Foundations
        for (int i = 0; i < 4; i++) {
            if (hands[i].getCardCount() != 0) {
                drawCard(hands[i].x, hands[i].y,
                         hands[i].getCard(0));
            } else {
                gb.display.setColor(PSEUDO_GRAY);
                gb.display.drawRect(hands[i].x, hands[i].y,
       10, 14);
            }
        }
    */
}

void drawPile(Pile *pile) {
    int baseIndex = max(0, pile->getCardCount() - MAX_CARDS_DRAWN_IN_PILE);
    for (int i = 0; i < min(pile->getCardCount(), MAX_CARDS_DRAWN_IN_PILE);
         i++) {
        drawCard(pile->x, pile->y + 2 * i,
                 pile->getCard(pile->getCardCount() - i - 1 - baseIndex));
    }
}

byte cardYPosition(Pile *pile, byte cardIndex) {
    /*if (pile->isTableau) {
        if (cardIndex > MAX_CARDS_DRAWN_IN_PILE - 1)
            return pile->y;
        return pile->y +
               2 * (min(pile->getCardCount(), MAX_CARDS_DRAWN_IN_PILE) -
                    cardIndex - 1);
    }
*/
    return pile->y;
}

void drawCard(byte x, byte y, Card card) {
    // Fill
    byte fill = WHITE;
    if (card.isFaceDown())
        fill = PSEUDO_GRAY;
    gb.display.setColor(fill);
    gb.display.fillRect(x + 1, y + 1, 8, 12);

    // Draw border
    gb.display.setColor(BLACK);
    gb.display.drawFastHLine(x + 1, y, 8);
    gb.display.drawFastHLine(x + 1, y + 13, 8);
    gb.display.drawFastVLine(x, y + 1, 12);
    gb.display.drawFastVLine(x + 9, y + 1, 12);

    if (card.isFaceDown())
        return;

    if (card.isRed())
        gb.display.setColor(GRAY);
    drawSuit(x + 2, y + 2, card.getSuit());
    gb.display.setColor(BLACK);
    drawValue(x + 5, y + 7, card.getValue());
}

void drawSuit(byte x, byte y, Suit suit) {
    switch (suit) {
    case spade:
        drawSpade(x, y);
        break;
    case club:
        drawClub(x, y);
        break;
    case heart:
        drawHeart(x, y);
        break;
    case diamond:
        drawDiamond(x, y);
        break;
    }
}

void drawLeftArrow(byte x, byte y) {
    gb.display.drawPixel(x, y + 2);
    gb.display.drawPixel(x + 1, y + 1);
    gb.display.drawPixel(x + 1, y + 3);
    gb.display.drawPixel(x + 2, y);
    gb.display.drawPixel(x + 2, y + 4);
}

void drawRightArrow(byte x, byte y) {
    gb.display.drawPixel(x, y);
    gb.display.drawPixel(x, y + 4);
    gb.display.drawPixel(x + 1, y + 1);
    gb.display.drawPixel(x + 1, y + 3);
    gb.display.drawPixel(x + 2, y + 2);
}

void drawHeart(byte x, byte y) {
    gb.display.drawPixel(x + 1, y);
    gb.display.drawPixel(x + 3, y);
    gb.display.drawFastHLine(x, y + 1, 5);
    gb.display.drawFastHLine(x, y + 2, 5);
    gb.display.drawFastHLine(x + 1, y + 3, 3);
    gb.display.drawPixel(x + 2, y + 4);
}

void drawDiamond(byte x, byte y) {
    gb.display.drawPixel(x + 2, y);
    gb.display.drawFastHLine(x + 1, y + 1, 3);
    gb.display.drawFastHLine(x, y + 2, 5);
    gb.display.drawFastHLine(x + 1, y + 3, 3);
    gb.display.drawPixel(x + 2, y + 4);
}

void drawSpade(byte x, byte y) {
    gb.display.drawPixel(x + 2, y);
    gb.display.drawFastHLine(x + 1, y + 1, 3);
    gb.display.drawFastHLine(x, y + 2, 5);
    gb.display.drawFastHLine(x, y + 3, 5);
    gb.display.drawPixel(x + 2, y + 4);
}

void drawClub(byte x, byte y) {
    gb.display.drawFastHLine(x + 1, y, 3);
    gb.display.drawFastHLine(x + 1, y + 2, 3);
    gb.display.drawFastVLine(x, y + 1, 3);
    gb.display.drawFastVLine(x + 4, y + 1, 3);
    gb.display.drawFastVLine(x + 2, y + 1, 4);
}

void drawValue(byte x, byte y, Value value) {
    switch (value) {
    case ace:
        drawAce(x, y);
        break;
    case two:
        drawTwo(x, y);
        break;
    case three:
        drawThree(x, y);
        break;
    case four:
        drawFour(x, y);
        break;
    case five:
        drawFive(x, y);
        break;
    case six:
        drawSix(x, y);
        break;
    case seven:
        drawSeven(x, y);
        break;
    case eight:
        drawEight(x, y);
        break;
    case nine:
        drawNine(x, y);
        break;
    case ten:
        drawTen(x, y);
        break;
    case jack:
        drawJack(x, y);
        break;
    case queen:
        drawQueen(x, y);
        break;
    case king:
        drawKing(x, y);
        break;
    }
}

void drawAce(byte x, byte y) {
    gb.display.drawPixel(x + 1, y);
    gb.display.drawFastVLine(x, y + 1, 4);
    gb.display.drawFastVLine(x + 2, y + 1, 4);
    gb.display.drawPixel(x + 1, y + 2);
}

void drawTwo(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentG(x, y);
    drawSegmentE(x, y);
    drawSegmentD(x, y);
}

void drawThree(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentG(x, y);
    drawSegmentC(x, y);
    drawSegmentD(x, y);
}

void drawFour(byte x, byte y) {
    drawSegmentF(x, y);
    drawSegmentG(x, y);
    drawSegmentB(x, y);
    drawSegmentC(x, y);
}

void drawFive(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentF(x, y);
    drawSegmentG(x, y);
    drawSegmentC(x, y);
    drawSegmentD(x, y);
}

void drawSix(byte x, byte y) {
    drawFive(x, y);
    drawSegmentE(x, y);
}

void drawSeven(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentC(x, y);
}

void drawEight(byte x, byte y) {
    drawSix(x, y);
    drawSegmentB(x, y);
}

void drawNine(byte x, byte y) {
    drawFour(x, y);
    drawSegmentA(x, y);
}

void drawTen(byte x, byte y) {
    drawSeven(x, y);
    drawSegmentD(x, y);
    drawSegmentE(x, y);
    drawSegmentF(x, y);
    gb.display.drawFastVLine(x - 2, y, 5);
}

void drawJack(byte x, byte y) {
    drawSegmentB(x, y);
    drawSegmentC(x, y);
    drawSegmentD(x, y);
    gb.display.drawPixel(x, y + 3);
}

void drawQueen(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentF(x, y);
    gb.display.drawFastHLine(x, y + 3, 3);
    gb.display.drawPixel(x + 1, y + 4);
}

void drawKing(byte x, byte y) {
    drawSegmentF(x, y);
    drawSegmentE(x, y);
    gb.display.drawPixel(x + 1, y + 2);
    gb.display.drawFastVLine(x + 2, y, 2);
    gb.display.drawFastVLine(x + 2, y + 3, 2);
}

void drawCursor() {
    bool flipped;
    byte x, y;
    getCursorDestination(x, y, flipped);

    cursorX = updatePosition(cursorX, x);
    cursorY = updatePosition(cursorY, y);

    drawCursor(cursorX, cursorY, flipped);
}

void getCursorDestination(byte &x, byte &y, bool &flipped) {
    Pile *pile = getActiveLocationPile();

    switch (activeLocation) {
    case stock:
        x = pile->x + 10;
        y = pile->y + 4;
        flipped = false;
        break;
    case table:
        x = pile->x + 4 * 11;
        y = pile->y + 4;
        flipped = false;
        break;
    case played:
        x = pile->x - 2;
        y = pile->y + 4;
        flipped = true;
        break;
    case hand:
        if (cardIndex == pile->getMaxVisibleCards() - 1) {
            x = pile->x - 7 + cardIndex * 11;
            flipped = true;
        } else {
            x = pile->x + 10 + cardIndex * 11;
            flipped = false;
        }
        y = pile->y + 4;
        break;
        /*case talon:
            x = pile->x + 10 + 2 * min(2, max(0, pile->getCardCount() -
        1)); y = pile->y + 4; flipped = false; break; case hand1: case
        hand2: case hand3: case hand4: x = pile->x - 7; y = pile->y + 4;
            flipped = true;
            break;
        case tableau1:
        case tableau2:
        case tableau3:
            x = pile->x + 10;
            y = (cardIndex == 0 ? 4 : -2) + cardYPosition(pile,
        cardIndex); flipped = false; break; case tableau4: case
        tableau5: case tableau6: case tableau7: x = pile->x - 7; y =
        (cardIndex == 0 ? 4 : -2) + cardYPosition(pile, cardIndex);
            flipped = true;
            break;*/
    }
}

void drawDealing() {
    if (cardAnimationCount < dealingCount && gb.frameCount % 4 == 0) {
        cardAnimationCount++;
        playSoundA();
    }
    bool doneDealing = cardAnimationCount == dealingCount;
    for (int i = 0; i < cardAnimationCount; i++) {
        if (cardAnimations[i].x != cardAnimations[i].destX ||
            cardAnimations[i].y != cardAnimations[i].destY) {
            doneDealing = false;
            drawCard(cardAnimations[i].x, cardAnimations[i].y,
                     cardAnimations[i].card);
            cardAnimations[i].x =
                updatePosition(cardAnimations[i].x, cardAnimations[i].destX);
            cardAnimations[i].y =
                updatePosition(cardAnimations[i].y, cardAnimations[i].destY);
            if (cardAnimations[i].x == cardAnimations[i].destX &&
                cardAnimations[i].y == cardAnimations[i].destY) {
                cardAnimations[i].destination->addCard(cardAnimations[i].card);
            }
        }
    }
    if (doneDealing)
        mode = selecting;
}

void drawDrawingCards() {
    /*
    drawPile(&moving);
    moving.x = updatePosition(moving.x, 17);
    moving.y = updatePosition(moving.y, 0);
    if (moving.x == 17 && moving.y == 0) {
        talonDeck.addCard(moving.getCard(0));
        if (remainingDraws) {
            remainingDraws--;
            moving.empty();
            drawAndFlip(&stockDeck, &moving);
            moving.x = stockDeck.x;
            moving.y = stockDeck.y;
            playSoundA();
        } else {
            UndoAction action;
            action.setDraw();
            undo.pushAction(action);
            mode = selecting;
        }
    }
        */
}

void drawMovingPile() {
    /*
    drawPile(&moving);
    Pile *pile = getActiveLocationPile();
    byte yDelta = 2;
    if (pile->isTableau)
        yDelta += 2 * pile->getCardCount();
    moving.x = updatePosition(moving.x, pile->x);
    moving.y = updatePosition(moving.y, pile->y + yDelta);
    */
}

void drawIllegalMove() {
    /*
    // Move the cards back to the source pile.
    byte yDelta = 0;
    if (sourcePile->isTableau)
        yDelta += 2 * sourcePile->getCardCount();
    moving.x = updatePosition(moving.x, sourcePile->x);
    moving.y = updatePosition(moving.y, sourcePile->y + yDelta);
    drawPile(&moving);
    // Check to see if the animation is done
    if (moving.x == sourcePile->x && moving.y == sourcePile->y + yDelta)
    { sourcePile->addPile(&moving); bool revealed = updateAfterPlay();

        // Update undo stack if this was a fast move to the hand.
        if (mode == fastFoundation) {
            UndoAction action;
            action.source = getActiveLocationPile();
            action.destination = sourcePile;
            action.setCardCount(1);
            if (revealed)
                action.setRevealed();
            undo.pushAction(action);
        }
        if (mode != gameOver)
            mode = selecting;
    }
            */
}

void drawWonGame() {
    // Bounce the cards from the hands, one at a time.
    if (!gb.display.persistence) {
        gb.display.persistence = true;
        drawBoard();
        initializeCardBounce();
    }

    // Apply gravity
    bounce.yVelocity += 0x0080;
    bounce.x += bounce.xVelocity;
    bounce.y += bounce.yVelocity;
    // If the card is at the bottom of the screen, reverse the y
    // velocity and scale by 80%.
    if (bounce.y + (14 << 8) > LCDHEIGHT << 8) {
        bounce.y = (LCDHEIGHT - 14) << 8;
        bounce.yVelocity = bounce.yVelocity * -4 / 5;
        playSoundB();
    }
    drawCard(bounce.x >> 8, bounce.y >> 8, bounce.card);
    // Check to see if the current card is off the screen.
    if (bounce.x + (10 << 8) < 0 || bounce.x > LCDWIDTH << 8) {
        if (!initializeCardBounce())
            showTitle();
    }
}

bool initializeCardBounce() {
    /*
    // Return false if all the cards are done.
    if (hands[bounceIndex].getCardCount() == 0)
        return false;
    // Pick the next card to animate, with a random initial velocity.
    bounce.card = hands[bounceIndex].removeTopCard();
    bounce.x = hands[bounceIndex].x << 8;
    bounce.y = hands[bounceIndex].y << 8;
    bounce.xVelocity = (random(2) ? 1 : -1) * random(0x0100, 0x0200);
    bounce.yVelocity = -1 * random(0x0200);
    bounceIndex = (bounceIndex + 1) % 4;
    */
    return true;
}

Pile *getActiveLocationPile() {
    switch (activeLocation) {
    case stock:
        return &stockDeck;
    case table:
        return &tableRow;
    case hand:
        return &playerDeck[0];
    case played:
        return &playedRow;
    }
}

byte updatePosition(byte current, byte destination) {
    if (current == destination)
        return current;

    byte delta = (destination - current) / 3;
    if (delta == 0 && ((gb.frameCount % 3) == 0))
        delta = destination > current ? 1 : -1;
    return current + delta;
}

void drawCursor(byte x, byte y, bool flipped) {
    if (flipped) {
        for (int i = 0; i < 4; i++) {
            gb.display.setColor(BLACK);
            gb.display.drawPixel(x + 3 + i, y + i);
            gb.display.drawPixel(x + 3 + i, y + (6 - i));
            gb.display.setColor(WHITE);
            gb.display.drawFastHLine(x + 3, y + i, i);
            gb.display.drawFastHLine(x + 3, y + (6 - i), i);
        }
        gb.display.setColor(BLACK);
        gb.display.drawFastVLine(x + 2, y, 7);
        gb.display.drawFastHLine(x, y + 2, 2);
        gb.display.drawFastHLine(x, y + 4, 2);
        gb.display.drawPixel(x, y + 3);
        gb.display.setColor(WHITE);
        gb.display.drawFastHLine(x + 1, y + 3, 2);
        if (false /*cardIndex != 0*/) {
            Card card = getActiveLocationPile()->getCard(cardIndex);
            byte extraWidth = card.getValue() == ten ? 2 : 0;
            gb.display.setColor(BLACK);
            gb.display.drawRect(x - 12 - extraWidth, y - 1, 13 + extraWidth, 9);
            gb.display.setColor(WHITE);
            gb.display.drawPixel(x, y + 3);
            gb.display.fillRect(x - 11 - extraWidth, y, 11 + extraWidth, 7);
            gb.display.setColor(card.isRed() ? PSEUDO_GRAY : BLACK);
            drawValue(x - 10, y + 1, card.getValue());
            drawSuit(x - 6, y + 1, card.getSuit());
        }
    } else {
        for (int i = 0; i < 4; i++) {
            gb.display.setColor(BLACK);
            gb.display.drawPixel(x + 3 - i, y + i);
            gb.display.drawPixel(x + 3 - i, y + (6 - i));
            gb.display.setColor(WHITE);
            gb.display.drawFastHLine(x + 4 - i, y + i, i);
            gb.display.drawFastHLine(x + 4 - i, y + (6 - i), i);
        }
        gb.display.setColor(BLACK);
        gb.display.drawFastVLine(x + 4, y, 7);
        gb.display.drawFastHLine(x + 5, y + 2, 2);
        gb.display.drawFastHLine(x + 5, y + 4, 2);
        gb.display.drawPixel(x + 6, y + 3);
        gb.display.setColor(WHITE);
        gb.display.drawFastHLine(x + 4, y + 3, 2);
        if (false /*cardIndex != 0*/) {
            Card card = getActiveLocationPile()->getCard(cardIndex);
            byte extraWidth = card.getValue() == ten ? 2 : 0;
            gb.display.setColor(BLACK);
            gb.display.drawRect(x + 6, y - 1, 13 + extraWidth, 9);
            gb.display.setColor(WHITE);
            gb.display.drawPixel(x + 6, y + 3);
            gb.display.fillRect(x + 7, y, 11 + extraWidth, 7);
            gb.display.setColor(card.isRed() ? PSEUDO_GRAY : BLACK);
            drawValue(x + 8 + extraWidth, y + 1, card.getValue());
            drawSuit(x + 12 + extraWidth, y + 1, card.getSuit());
        }
    }
}

void drawSegmentA(byte x, byte y) {
    gb.display.drawFastHLine(x, y, 3);
}

void drawSegmentB(byte x, byte y) {
    gb.display.drawFastVLine(x + 2, y, 3);
}

void drawSegmentC(byte x, byte y) {
    gb.display.drawFastVLine(x + 2, y + 2, 3);
}

void drawSegmentD(byte x, byte y) {
    gb.display.drawFastHLine(x, y + 4, 3);
}

void drawSegmentE(byte x, byte y) {
    gb.display.drawFastVLine(x, y + 2, 3);
}

void drawSegmentF(byte x, byte y) {
    gb.display.drawFastVLine(x, y, 3);
}

void drawSegmentG(byte x, byte y) {
    gb.display.drawFastHLine(x, y + 2, 3);
}

void playSoundA() {
    gb.sound.playPattern(patternA, 0);
}

void playSoundB() {
    gb.sound.playPattern(patternB, 0);
}

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
        address += loadPile(address, &stockDeck);
        /*address += loadPile(address, &talonDeck);
        for (int i = 0; i < 4; i++)
            address += loadPile(address, &hands[i]);
        for (int i = 0; i < 7; i++)
            address += loadPile(address, &tableau[i]);*/
    } else {
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
        address += savePile(address, &stockDeck);
        /*address += savePile(address, &talonDeck);
        for (int i = 0; i < 4; i++)
            address += savePile(address, &hands[i]);
        for (int i = 0; i < 7; i++)
            address += savePile(address, &tableau[i]);*/
    }
}

int savePile(int address, Pile *pile) {
    EEPROM.put(address, pile->getCardCount());
    for (int i = 0; i < pile->getMaxCards(); i++) {
        if (pile->getCardCount() > i) {
            EEPROM.put(address + i + 1,
                       pile->getCard(pile->getCardCount() - i - 1));
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

void displayStatistics() {
    while (true) {
        if (gb.update()) {
            gb.display.cursorX = 0;
            gb.display.cursorY = 0;
            gb.display.print(F("Easy started: "));
            gb.display.println(easyGameCount);
            gb.display.print(F("Easy won:     "));
            gb.display.println(easyGamesWon);
            gb.display.print(F("Hard started: "));
            gb.display.println(hardGameCount);
            gb.display.print(F("Hard won:     "));
            gb.display.println(hardGamesWon);

            if (gb.buttons.pressed(BTN_A) || gb.buttons.pressed(BTN_B) ||
                gb.buttons.pressed(BTN_C))
                return;
        }
    }
}

void performUndo() {
    /*
    // Make sure there is something to undo.
    if (!undo.isEmpty() && mode == selecting) {
        UndoAction action = undo.popAction();
        // Handle draw from stock.
        if (action.wasDraw()) {
            for (byte i = 0; i < cardsToDraw; i++) {
                drawAndFlip(&talonDeck, &stockDeck);
            }
        }
        // Handle flipped talon.
        else if (action.wasFlippedTalon()) {
            while (stockDeck.getCardCount() != 0) {
                drawAndFlip(&stockDeck, &talonDeck);
            }
        }
        // Handle moving cards from one pile to another.
        else {
            // Handle moved cards resulted in revealing another card.
            if (action.wasRevealed()) {
                drawAndFlip(action.source, action.source);
            }
            moving.empty();
            action.destination->removeCards(action.getCardCount(),
    &moving); action.source->addPile(&moving);
        }
        updateAfterPlay();
    }
        */
}

void debug(unsigned char c) {
    gb.display.drawChar(35, 1, c, 1);
}
