#include "card.h"
#include <Gamebuino.h>

#ifndef PILE_H
#define PILE_H
class Pile {
  public:
    Pile(byte maxCards, byte maxVisible);
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
    byte cardCount;
    bool scrollToLast;

  private:
    Card *_cards;
    byte _maxCards;
};
#endif
