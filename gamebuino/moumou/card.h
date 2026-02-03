#include <Gamebuino.h>

#ifndef CARD_H
#define CARD_H
enum Suit {
    spade = 0,
    club,
    heart,
    diamond
};
enum Value {
    undef = 1,
    two,
    three,
    four,
    five,
    six,
    seven,
    eight,
    nine,
    ten,
    jack,
    queen,
    king,
    ace
};

class Card {
  public:
    Card() : Card(undef, spade, false) {
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
#endif
