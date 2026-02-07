#include <Gamebuino.h>

#ifndef CARD_H
#define CARD_H
#define Undefined UINT8_MAX

typedef enum {
    Spades = 0,
    Hearts,
    Diamonds,
    Clubs,
    SuitCount
} Suit;

typedef enum {
    Six = 0,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace,
    ValueCount
} Value;

class Card {
  public:
    Card() : Card(Undefined, Spades, false) {
    }
    Card(Value value, Suit suit, bool faceDown);
    bool isFaceDown() const;
    Value getValue() const;
    Suit getSuit() const;
    bool isRed() const;
    void setFace(bool up);

  private:
    byte _value;
};

Value CardValue(Card card);
Suit CardSuit(Card card);

#endif
