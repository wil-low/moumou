#include <EEPROM.h>
#include <Gamebuino.h>
#include <SPI.h>

#include "core.h"

#define MAX_CARDS_DRAWN_IN_PILE 10
#define EEPROM_MAGIC_NUMBER 171

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

GameState gameState;

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
const char *const pauseMenu[5] PROGMEM = {resumeOption, quitOption,
                                          statisticsOption, saveOption};

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

    GameState *state = &gameState;

    // Initialize positions of piles.
    gameState._deck.x = 1;
    gameState._deck.y = 0;
    gameState._deck.maxVisibleCards = 1;
    gameState._deck.faceUp = false;

    gameState._table.x = 16;
    gameState._table.y = 0;
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
    gameState._players[1]._hand.y = 0;
    gameState._players[1]._hand.maxVisibleCards = 1;
    gameState._players[1]._hand.faceUp = false;

    gameState._players[0]._score = 0;
    gameState._players[1]._score = 0;

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
    switch (gb.menu(pauseMenu, mode == selecting ? 4 : 3)) {
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
    case 0:
    default:
        // Resume the game
        break;
    }
}

void setupNewGame() {
    activeLocation = stock;
    cardIndex = 0;
    cursorX = 11;
    cursorY = 4;

    gameState._deck.newDeck();
    gameState._deck.shuffle();

    gameState._players[0]._hand.empty();
    gameState._players[1]._hand.empty();
    gameState._cur_player = 0;

    gameState._played.empty();
    gameState._table.empty();

    // Initialize the data structure to deal out the initial board.
    cardAnimationCount = 0;
    for (int i = 0; i < INITIAL_HAND; i++)
        for (int p = 0; p < 2; p++)
            animateMove(&gameState._deck, 0, &gameState._players[p]._hand, i);
    dealingCount = cardAnimationCount;
    cardAnimationCount = 0;
    mode = initialDealing;
}

void animateMove(Pile *src, byte srcIdx, Pile *dst, byte dstIdx) {
    Card card = src->removeCardAt(srcIdx);
    card.setFace(dst->faceUp);
    CardAnimation ca;
    ca.x = src->x + dst->getCardPosition(srcIdx) * 11;
    ca.y = src->y;
    ca.destX = dst->x + dst->getCardPosition(dstIdx) * 11;
    ca.destY = dst->y;
    ca.destination = dst;
    ca.card = card;
    cardAnimations[cardAnimationCount++] = ca;
}

const uint16_t patternA[] PROGMEM = {0x0045, 0x0118, 0x0000};
const uint16_t patternB[] PROGMEM = {0x0045, 0x0108, 0x0000};

void handleSelectingButtons() {
    // Handle buttons when user is using the arrow cursor to navigate.
    Location originalLocation = activeLocation;
    if (gb.buttons.pressed(BTN_RIGHT)) {
        if (activeLocation == hand) {
            if (cardIndex + gameState._players[0]._hand.scrollOffset <
                gameState._players[0]._hand._count - 1) {
                if (cardIndex < gameState._players[0]._hand.maxVisibleCards - 1)
                    ++cardIndex;
                else
                    ++gameState._players[0]._hand.scrollOffset;
            }
        }
    }
    if (gb.buttons.pressed(BTN_LEFT)) {
        if (activeLocation == hand) {
            if (cardIndex > 0)
                --cardIndex;
            else if (gameState._players[0]._hand.scrollOffset > 0)
                --gameState._players[0]._hand.scrollOffset;
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
        if (gameState._deck._count != 0) {
            // drawAndFlip(&gameState._deck, &gameState._players[0]._hand);
            cardAnimationCount = 0;
            animateMove(&gameState._deck, 0, &gameState._players[1]._hand,
                        gameState._players[1]._hand._count);
            dealingCount = cardAnimationCount;
            cardAnimationCount = 0;
            mode = moving;
            // playSoundA();
        }
    } else if (gb.buttons.pressed(BTN_A)) {

        switch (activeLocation) {
        case stock:
            if (gameState._deck._count != 0) {
                cardAnimationCount = 0;
                for (int i = 0; i < 1; i++)
                    animateMove(&gameState._deck, 0,
                                &gameState._players[0]._hand,
                                gameState._players[0]._hand._count);
                dealingCount = cardAnimationCount;
                cardAnimationCount = 0;
                mode = moving;
                // playSoundA();
            } else {
                /*while (talonDeck._count != 0) {
                    drawAndFlip(&talonDeck, &gameState._deck);
                }
                UndoAction action;
                action.setFlippedTalon();
                undo.pushAction(action);*/
            }
            break;
        case hand:
            if (gameState._players[0]._hand._count) {
                cardAnimationCount = 0;
                animateMove(&gameState._players[0]._hand,
                            cardIndex +
                                gameState._players[0]._hand.scrollOffset,
                            &gameState._played, gameState._played._count);
                dealingCount = cardAnimationCount;
                cardAnimationCount = 0;
                if (gameState._players[0]._hand._count -
                        gameState._players[0]._hand.scrollOffset >=
                    gameState._players[0]._hand.maxVisibleCards) {
                } else if (gameState._players[0]._hand.scrollOffset > 0) {
                    gameState._players[0]._hand.scrollOffset--;
                } else if (cardIndex == gameState._players[0]._hand._count &&
                           cardIndex > 0) {
                    cardIndex--;
                }
                mode = moving;
            }
            break;
        case played:
            cardAnimationCount = 0;
            byte count = gameState._played._count;
            for (int i = 0; i < count; i++)
                animateMove(&gameState._played, 0, &gameState._table,
                            gameState._table._count + i);
            dealingCount = cardAnimationCount;
            cardAnimationCount = 0;
            mode = moving;
            break;
        }
    }
    if (originalLocation != activeLocation)
        cardIndex = 0;
}

void checkWonGame() {
    /*
    // Check to see if all hands are full
    if (hands[0]._count == 13 &&
        hands[1]._count == 13 &&
        hands[2]._count == 13 &&
        hands[3]._count == 13) {
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

void drawDeck(Pile *deck, bool showCount) {
    for (int i = 0; i < deck->maxVisibleCards; ++i) {
        if (i + deck->scrollOffset >= deck->_count)
            break;
        if (deck->faceUp) {
            drawCard(
                deck->x + i * 11, deck->y,
                deck->getCard(deck->_count - (i + deck->scrollOffset) - 1));
        } else {
            drawCard(deck->x, deck->y, Card(Undefined, Spades, true));
        }
    }

    if (showCount && deck->_count && !deck->faceUp) {
        gb.display.setColor(WHITE);
        drawNumberRight(deck->_count, deck->x + 8, deck->y + 2);
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
    drawDeck(&gameState._deck, false);

    // Scores
    drawNumberRight(gameState._played._count, 83, 17);
    // drawNumberRight(gameState._players[1]._score, 83, 17);
    drawNumberRight(gameState._players[0]._score, 83, 25);

    // Bot deck
    drawDeck(&gameState._players[1]._hand, true);

    // Human deck
    drawDeck(&gameState._players[0]._hand, false);
    if (gameState._players[1]._hand._count != 0) {
        drawCard(gameState._players[1]._hand.x, gameState._players[1]._hand.y,
                 Card(Undefined, Spades, true));
        gb.display.setColor(WHITE);
        drawNumberRight(gameState._players[1]._hand._count, 81, 2);
    }

    drawDeck(&gameState._players[0]._hand, false);
    if (gameState._players[0]._hand.scrollOffset > 0)
        drawLeftArrow(0, 38);
    if (gameState._players[0]._hand._count -
            gameState._players[0]._hand.scrollOffset >
        gameState._players[0]._hand.maxVisibleCards)
        drawRightArrow(81, 38);

    drawDeck(&gameState._played, false);
    drawDeck(&gameState._table, false);
}

void drawPile(Pile *pile) {
    int baseIndex = max(0, pile->_count - MAX_CARDS_DRAWN_IN_PILE);
    for (int i = 0; i < min(pile->_count, MAX_CARDS_DRAWN_IN_PILE); i++) {
        drawCard(pile->x, pile->y + 2 * i,
                 pile->getCard(pile->_count - i - 1 - baseIndex));
    }
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
    case Spades:
        drawSpade(x, y);
        break;
    case Clubs:
        drawClub(x, y);
        break;
    case Hearts:
        drawHeart(x, y);
        break;
    case Diamonds:
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
    case Ace:
        drawAce(x, y);
        break;
        /*    case Two:
                drawTwo(x, y);
                break;
            case Three:
                drawThree(x, y);
                break;
            case Four:
                drawFour(x, y);
                break;
            case Five:
                drawFive(x, y);
                break;*/
    case Six:
        drawSix(x, y);
        break;
    case Seven:
        drawSeven(x, y);
        break;
    case Eight:
        drawEight(x, y);
        break;
    case Nine:
        drawNine(x, y);
        break;
    case Ten:
        drawTen(x, y);
        break;
    case Jack:
        drawJack(x, y);
        break;
    case Queen:
        drawQueen(x, y);
        break;
    case King:
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
        if (cardIndex == pile->maxVisibleCards - 1) {
            x = pile->x - 7 + cardIndex * 11;
            flipped = true;
        } else {
            x = pile->x + 10 + cardIndex * 11;
            flipped = false;
        }
        y = pile->y + 4;
        break;
    }
}

void drawDealing() {
    if (cardAnimationCount < dealingCount && gb.frameCount % 4 == 0) {
        cardAnimationCount++;
        playSoundA();
    }
    bool doneDealing = cardAnimationCount == dealingCount;
    for (int i = 0; i < cardAnimationCount; i++) {
        auto ca = &cardAnimations[i];
        if (ca->x != ca->destX || ca->y != ca->destY) {
            doneDealing = false;
            drawCard(ca->x, ca->y, ca->card);
            ca->x = updatePosition(ca->x, ca->destX);
            ca->y = updatePosition(ca->y, ca->destY);
            if (ca->x == ca->destX && ca->y == ca->destY) {
                ca->destination->addCard(ca->card);
            }
        }
    }
    if (doneDealing)
        mode = selecting;
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
    if (hands[bounceIndex]._count == 0)
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
        return &gameState._deck;
    case table:
        return &gameState._table;
    case hand:
        return &gameState._players[0]._hand;
    case played:
        return &gameState._played;
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
            byte extraWidth = card.getValue() == Ten ? 2 : 0;
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
            byte extraWidth = card.getValue() == Ten ? 2 : 0;
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
        address += loadPile(address, &gameState._deck);
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
        address += savePile(address, &gameState._deck);
        /*address += savePile(address, &talonDeck);
        for (int i = 0; i < 4; i++)
            address += savePile(address, &hands[i]);
        for (int i = 0; i < 7; i++)
            address += savePile(address, &tableau[i]);*/
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

void debug(unsigned char c) {
    gb.display.drawChar(35, 1, c, 1);
}
