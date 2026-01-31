require("core")

Card = {}
Card.__index = Card

Card.SUITS = "SHDC"
Card.VALUES = "6789TJQKA"

function Card:__tostring()
    return string.sub(Card.VALUES, self.value, self.value)
        .. string.sub(Card.SUITS, self.suit, self.suit)
end

function Card.init(suit, value, x, y)
    local self = setmetatable({}, Card)
    self.suit = suit
    self.value = value
    self.image = love.graphics.newImage("img/" .. tostring(self) .. ".png")
    self.x = x
    self.y = y
    self.faceUp = false
    return self
end

function Card:draw()
    love.graphics.draw(self.faceUp and self.image or Deck.back, self.x, self.y, 0, scale, scale, 0)
end
