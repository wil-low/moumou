require("core")
require("card")

Deck = {}
Deck.__index = Deck

Deck.back = love.graphics.newImage("img/red_back.png")

function Deck.init(label, x, y, faceUp)
    local self = setmetatable({}, Deck)
    self.label = label
    self.x = x
    self.y = y
    self.items = {}
    self.faceUp = faceUp
    return self
end

function Deck:createCards()
    self.items = {}
    for v = 1, Value.Count do
        for s = 1, Suit.Count do
            self.items[#self.items + 1] = Card.init(s, v, #self.items * 20, 0)
        end
    end
end

function Deck:clear()
    self.items = {}
end

function Deck:deal()
    self.items = {}
end

function Deck:cardCoords(idx)
    return self.x + (idx - 1) * slot_x, self.y
end

function Deck:draw()
    love.graphics.setColor(1, 1, 1)
    if self.faceUp then
        for i, card in ipairs(self.items) do
            local x, y = self:cardCoords(i)
            love.graphics.draw(card.image, x, y, 0, scale, scale, 0)
        end
    else
        love.graphics.draw(Deck.back, self.x, self.y, 0, scale, scale, 0)
    end
    love.graphics.setColor(0, 0, 0)
    --love.graphics.print(self.label, self.x, self.y + slot_y)
end
