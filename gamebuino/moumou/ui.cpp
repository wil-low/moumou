#include "ui.h"
#include "config.h"
#include <Gamebuino.h>

#define PSEUDO_GRAY GRAY
// #define PSEUDO_GRAY BLACK

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

extern GameState gameState;

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

extern Gamebuino gb;

void UI::showTitle() {
start:
    gb.display.persistence = true;
    gb.titleScreen(F(""), title);
    if (RANDOM_SEED)
        randomSeed(RANDOM_SEED);
    else
        gb.pickRandomSeed();
    gb.battery.show = false;
    new_round(&gameState, this);
    /*readEeprom();

    // If there is a saved game in EEPROM, just skip right to the game.
    if (continueGame) {
        writeEeprom(false);
        mode = selecting;
        return;
    }*/

    // Ask whether we want easy (flip 1 card per draw) or hard (flip 3 cards per
    // draw).
    char menuOption;
askAgain:
    menuOption = gb.menu(newGameMenu, 3);
    if (menuOption == -1)
        goto start;

    if (menuOption == 0) {
        _botLevel = 1;
        _easyGameCount++;
        // writeEeprom(false);
    } else if (menuOption == 1) {
        _botLevel = 2;
        _hardGameCount++;
        // writeEeprom(false);
    } else {
        displayStatistics();
        goto askAgain;
    }
}

void UI::pause() {
askAgain:
    switch (gb.menu(pauseMenu, _mode == selecting ? 4 : 3)) {
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
        // writeEeprom(true);
        showTitle();
        break;
    case 0:
    default:
        // Resume the game
        break;
    }
}

void UI::animateMove(Pile *src, byte srcIdx, Pile *dst, byte dstIdx) {
    Card card = src->removeCardAt(srcIdx);
    card.setFace(dst->faceUp);
    CardAnimation ca;
    ca.x = src->x + dst->getCardPosition(srcIdx) * 11;
    ca.y = src->y;
    ca.destX = dst->x + dst->getCardPosition(dstIdx) * 11;
    ca.destY = dst->y;
    ca.destination = dst;
    ca.card = card;
    _cardAnimations[_cardAnimationCount++] = ca;
}

const uint16_t patternA[] PROGMEM = {0x0045, 0x0118, 0x0000};
const uint16_t patternB[] PROGMEM = {0x0045, 0x0108, 0x0000};

void UI::drawBoard() {
    // Stock
    drawDeck(&gameState._deck, false);

    if (gameState._valid_moves._draw)
        drawAllowedMove(gameState._deck.x, gameState._deck.y);

    // Scores
    drawNumberRight(gameState._cur_player, 83, 17);
    // drawNumberRight(gameState._fvm_calls, 83, 17);
    drawNumberRight(gameState._input_cmd, 83, 25);
    // drawNumberRight(gameState._players[1]._score, 83, 17);
    // drawNumberRight(gameState._players[0]._score, 83, 25);

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

    uint8_t idx = 0;
    for (uint8_t i = 0; i < gameState._players[0]._hand._count; ++i) {
        if (idx < gameState._valid_moves._count &&
            i == gameState._valid_moves._items[idx]) {
            drawAllowedMove(gameState._players[0]._hand.x + 11 * i,
                            gameState._players[0]._hand.y);
            idx++;
        }
    }

    drawDeck(&gameState._played, false);
    if (gameState._played._count && gameState._valid_moves._pass)
        drawAllowedMove(gameState._played.x, gameState._played.y);

    drawDeck(&gameState._table, false);
}

void UI::drawDeck(Pile *deck, bool showCount) {
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

void UI::drawCard(byte x, byte y, Card card) {
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

void UI::drawSuit(byte x, byte y, Suit suit) {
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

void UI::drawLeftArrow(byte x, byte y) {
    gb.display.drawPixel(x, y + 2);
    gb.display.drawPixel(x + 1, y + 1);
    gb.display.drawPixel(x + 1, y + 3);
    gb.display.drawPixel(x + 2, y);
    gb.display.drawPixel(x + 2, y + 4);
}

void UI::drawRightArrow(byte x, byte y) {
    gb.display.drawPixel(x, y);
    gb.display.drawPixel(x, y + 4);
    gb.display.drawPixel(x + 1, y + 1);
    gb.display.drawPixel(x + 1, y + 3);
    gb.display.drawPixel(x + 2, y + 2);
}

void UI::drawHeart(byte x, byte y) {
    gb.display.drawPixel(x + 1, y);
    gb.display.drawPixel(x + 3, y);
    gb.display.drawFastHLine(x, y + 1, 5);
    gb.display.drawFastHLine(x, y + 2, 5);
    gb.display.drawFastHLine(x + 1, y + 3, 3);
    gb.display.drawPixel(x + 2, y + 4);
}

void UI::drawDiamond(byte x, byte y) {
    gb.display.drawPixel(x + 2, y);
    gb.display.drawFastHLine(x + 1, y + 1, 3);
    gb.display.drawFastHLine(x, y + 2, 5);
    gb.display.drawFastHLine(x + 1, y + 3, 3);
    gb.display.drawPixel(x + 2, y + 4);
}

void UI::drawSpade(byte x, byte y) {
    gb.display.drawPixel(x + 2, y);
    gb.display.drawFastHLine(x + 1, y + 1, 3);
    gb.display.drawFastHLine(x, y + 2, 5);
    gb.display.drawFastHLine(x, y + 3, 5);
    gb.display.drawPixel(x + 2, y + 4);
}

void UI::drawClub(byte x, byte y) {
    gb.display.drawFastHLine(x + 1, y, 3);
    gb.display.drawFastHLine(x + 1, y + 2, 3);
    gb.display.drawFastVLine(x, y + 1, 3);
    gb.display.drawFastVLine(x + 4, y + 1, 3);
    gb.display.drawFastVLine(x + 2, y + 1, 4);
}

void UI::drawValue(byte x, byte y, Value value) {
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

void UI::drawAce(byte x, byte y) {
    gb.display.drawPixel(x + 1, y);
    gb.display.drawFastVLine(x, y + 1, 4);
    gb.display.drawFastVLine(x + 2, y + 1, 4);
    gb.display.drawPixel(x + 1, y + 2);
}

void UI::drawTwo(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentG(x, y);
    drawSegmentE(x, y);
    drawSegmentD(x, y);
}

void UI::drawThree(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentG(x, y);
    drawSegmentC(x, y);
    drawSegmentD(x, y);
}

void UI::drawFour(byte x, byte y) {
    drawSegmentF(x, y);
    drawSegmentG(x, y);
    drawSegmentB(x, y);
    drawSegmentC(x, y);
}

void UI::drawFive(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentF(x, y);
    drawSegmentG(x, y);
    drawSegmentC(x, y);
    drawSegmentD(x, y);
}

void UI::drawSix(byte x, byte y) {
    drawFive(x, y);
    drawSegmentE(x, y);
}

void UI::drawSeven(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentC(x, y);
}

void UI::drawEight(byte x, byte y) {
    drawSix(x, y);
    drawSegmentB(x, y);
}

void UI::drawNine(byte x, byte y) {
    drawFour(x, y);
    drawSegmentA(x, y);
}

void UI::drawTen(byte x, byte y) {
    drawSeven(x, y);
    drawSegmentD(x, y);
    drawSegmentE(x, y);
    drawSegmentF(x, y);
    gb.display.drawFastVLine(x - 2, y, 5);
}

void UI::drawJack(byte x, byte y) {
    drawSegmentB(x, y);
    drawSegmentC(x, y);
    drawSegmentD(x, y);
    gb.display.drawPixel(x, y + 3);
}

void UI::drawQueen(byte x, byte y) {
    drawSegmentA(x, y);
    drawSegmentB(x, y);
    drawSegmentF(x, y);
    gb.display.drawFastHLine(x, y + 3, 3);
    gb.display.drawPixel(x + 1, y + 4);
}

void UI::drawKing(byte x, byte y) {
    drawSegmentF(x, y);
    drawSegmentE(x, y);
    gb.display.drawPixel(x + 1, y + 2);
    gb.display.drawFastVLine(x + 2, y, 2);
    gb.display.drawFastVLine(x + 2, y + 3, 2);
}

void UI::drawCursor() {
    bool flipped;
    byte x, y;
    getCursorDestination(x, y, flipped);

    _cursorX = updatePosition(_cursorX, x);
    _cursorY = updatePosition(_cursorY, y);

    drawCursor(_cursorX, _cursorY, flipped);
}

void UI::getCursorDestination(byte &x, byte &y, bool &flipped) {
    Pile *pile = getActiveLocationPile();

    switch (_activeLocation) {
    case stock:
        x = pile->x - 2;
        y = pile->y + 4;
        flipped = true;
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
        if (_cardIndex == pile->maxVisibleCards - 1) {
            x = pile->x - 7 + _cardIndex * 11;
            flipped = true;
        } else {
            x = pile->x + 10 + _cardIndex * 11;
            flipped = false;
        }
        y = pile->y + 4;
        break;
    }
}

void UI::drawDealing() {
    if (_cardAnimationCount < _dealingCount && gb.frameCount % 4 == 0) {
        _cardAnimationCount++;
        playSoundA();
    }
    bool doneDealing = _cardAnimationCount == _dealingCount;
    for (int i = 0; i < _cardAnimationCount; i++) {
        auto ca = &_cardAnimations[i];
        if (ca->x != ca->destX || ca->y != ca->destY) {
            doneDealing = false;
            drawCard(ca->x, ca->y, ca->card);
            ca->x = updatePosition(ca->x, ca->destX);
            ca->y = updatePosition(ca->y, ca->destY);
            if (ca->x == ca->destX && ca->y == ca->destY) {
                ca->destination->addCard(ca->card);
                gameState._last_card = ca->card;
            }
        }
    }
    if (doneDealing) {
        _mode = selecting;
        process_input(&gameState, this);
        find_valid_moves(&gameState, gameState._cur_player);
    }
}

void UI::drawWonGame() {
    /*
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
            */
}

void UI::displayStatistics() {
    while (true) {
        if (gb.update()) {
            gb.display.cursorX = 0;
            gb.display.cursorY = 0;
            gb.display.print(F("Easy started: "));
            gb.display.println(_easyGameCount);
            gb.display.print(F("Easy won:     "));
            gb.display.println(_easyGamesWon);
            gb.display.print(F("Hard started: "));
            gb.display.println(_hardGameCount);
            gb.display.print(F("Hard won:     "));
            gb.display.println(_hardGamesWon);

            if (gb.buttons.pressed(BTN_A) || gb.buttons.pressed(BTN_B) ||
                gb.buttons.pressed(BTN_C))
                return;
        }
    }
}

void UI::debug(uint8_t n0, uint8_t n1) {
    drawNumberRight(n0, 83, 33);
    drawNumberRight(n1, 83, 41);
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

Pile *UI::getActiveLocationPile() {
    switch (_activeLocation) {
    case stock:
        return &gameState._deck;
    case table:
        return &gameState._table;
    case hand:
        return &gameState._players[0]._hand;
    case played:
        return &gameState._played;
    }
    return nullptr;
}

byte UI::updatePosition(byte current, byte destination) {
    if (current == destination)
        return current;

    byte delta = (destination - current) / 3;
    if (delta == 0 && ((gb.frameCount % 3) == 0))
        delta = destination > current ? 1 : -1;
    return current + delta;
}

void UI::drawCursor(byte x, byte y, bool flipped) {
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
            Card card = getActiveLocationPile()->getCard(_cardIndex);
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
            Card card = getActiveLocationPile()->getCard(_cardIndex);
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

void UI::drawSegmentA(byte x, byte y) {
    gb.display.drawFastHLine(x, y, 3);
}

void UI::drawSegmentB(byte x, byte y) {
    gb.display.drawFastVLine(x + 2, y, 3);
}

void UI::drawSegmentC(byte x, byte y) {
    gb.display.drawFastVLine(x + 2, y + 2, 3);
}

void UI::drawSegmentD(byte x, byte y) {
    gb.display.drawFastHLine(x, y + 4, 3);
}

void UI::drawSegmentE(byte x, byte y) {
    gb.display.drawFastVLine(x, y + 2, 3);
}

void UI::drawSegmentF(byte x, byte y) {
    gb.display.drawFastVLine(x, y, 3);
}

void UI::drawSegmentG(byte x, byte y) {
    gb.display.drawFastHLine(x, y + 2, 3);
}

void UI::drawAllowedMove(byte x, byte y) {
    if (gameState._cur_player == 0) {
        gb.display.setColor(WHITE);
        gb.display.drawFastVLine(x + 6, y - 1, 4);
        gb.display.drawFastHLine(x + 7, y + 2, 3);
        gb.display.drawPixel(x + 8, y);
        gb.display.setColor(BLACK);
        gb.display.drawRect(x + 7, y - 1, 3, 3);
    }
}

void UI::playSoundA() {
    gb.sound.playPattern(patternA, 0);
}

void UI::playSoundB() {
    gb.sound.playPattern(patternB, 0);
}

void UI::drawNumberRight(uint16_t n, byte x, byte y) {
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
