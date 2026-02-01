#include "card.h"
#include <Gamebuino.h>

#ifndef PILE_H
#define PILE_H
class Pile {
  public:
    Pile(byte maxCards, byte maxVisibleCards);
    ~Pile();
    void addCard(Card card);
    void addPile(Pile *pile);
    byte getCardCount() const;
    Card getCard(int indexFromTop) const;
    Card removeCardAt(byte idx);
    void removeCards(int count, Pile *destination);
    void empty();
    void shuffle();
    void newDeck();
    byte getMaxCards() const;
    byte getMaxVisibleCards() const;
    byte x, y;
    byte cardOffset;

  private:
    Card *_cards;
    byte _maxCards;
    byte _maxVisibleCards;
    byte _count;
};
#endif
