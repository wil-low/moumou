#include "pile.h"

Pile::Pile(byte maxCards, byte maxVisible) {
    cardCount = 0;
    _maxCards = maxCards;
    maxVisibleCards = maxVisible;
    _cards = new Card[_maxCards];
    scrollOffset = 0;
    faceUp = true;
    scrollToLast = false;
}

Pile::~Pile() {
    delete[] _cards;
}

void Pile::addCard(Card card) {
    if (cardCount < _maxCards) {
        _cards[cardCount++] = card;
        if (scrollToLast)
            scrollOffset = max(0, cardCount - maxVisibleCards);
    }
}

void Pile::addPile(Pile *pile) {
    for (int i = pile->cardCount - 1; i >= 0; i--) {
        addCard(pile->getCard(i));
    }
}

Card Pile::getCard(int indexFromTop) const {
    if (indexFromTop < cardCount) {
        return _cards[cardCount - indexFromTop - 1];
    }
    return Card();
}

Card Pile::removeCardAt(byte idx) {
    if (idx < cardCount) {
        Card card = _cards[idx];
        --cardCount;
        for (byte i = idx; i < cardCount; i++)
            _cards[i] = _cards[i + 1];
        if (scrollToLast)
            scrollOffset = max(0, cardCount - maxVisibleCards);
        return card;
    }
    return Card();
}

void Pile::removeCards(int count, Pile *destination) {
    count = min(count, cardCount);
    cardCount -= count;
    for (int i = 0; i < count; i++)
        destination->addCard(_cards[cardCount + i]);
    if (scrollToLast)
        scrollOffset = max(0, cardCount - maxVisibleCards);
}

void Pile::empty() {
    cardCount = 0;
    scrollOffset = 0;
}

void Pile::shuffle() {
    for (int i = 0; i < cardCount; i++) {
        int randomIndex = random(cardCount - i);
        Card tmp = _cards[randomIndex];
        _cards[randomIndex] = _cards[cardCount - i - 1];
        _cards[cardCount - i - 1] = tmp;
    }
}

void Pile::newDeck() {
    empty();
    for (int suit = spade; suit <= diamond; suit++) {
        for (int value = six; value <= ace; value++) {
            addCard(
                Card(static_cast<Value>(value), static_cast<Suit>(suit), true));
        }
    }
}

byte Pile::getMaxCards() const {
    return _maxCards;
}

byte Pile::getCardPosition(int indexFromTop) const {
    if (indexFromTop < scrollOffset)
        return 0;
    if (indexFromTop >= scrollOffset + maxVisibleCards)
        return faceUp ? maxVisibleCards - 1 : 0;
    return indexFromTop - scrollOffset;
}
