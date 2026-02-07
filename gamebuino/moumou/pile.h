#include "card.h"
#include <Gamebuino.h>

#ifndef PILE_H
#define PILE_H
class Pile {
  public:
    Pile();
    ~Pile();
    void addCard(Card card);
    void addPile(Pile *pile);
    Card getCard(int indexFromTop) const;
    Card removeCardAt(byte idx);
    void removeCards(int count, Pile *destination);
    void empty();
    void shuffle();
    void newDeck();
    byte getMaxCards() const;
    byte getCardPosition(int indexFromTop) const;

    byte x, y;
    byte scrollOffset;
    bool faceUp;
    byte maxVisibleCards;
    byte _count;
    bool scrollToLast;

    static const byte _maxCards = 36;
    Card _items[_maxCards];
};
#endif
