require("core")
require("card")

Deck = {}
Deck.__index = Deck

Deck.back = love.graphics.newImage("img/red_back.png")

function Deck.init(label, x, y, faceUp, maxWidth)
    local self = setmetatable({}, Deck)
    self.label = label
    self.x = x
    self.y = y
    self.items = {}
    self.faceUp = faceUp
    self.maxWidth = maxWidth
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

function Deck:cardCoords(idx)
    local spacing = slot_x
    local w = Deck.back:getWidth() * scale
    if self.maxWidth ~= nil and #self.items > 1 then
        spacing = math.min(spacing, (self.maxWidth - w) / (#self.items - 1))
        if idx < #self.items then
            w = math.min(spacing, w)
        end
    end
    return self.x + (idx - 1) * spacing, self.y, w, Deck.back:getHeight() * scale
end

function Deck:draw()
    if #self.items > 0 then
        if self.maxWidth ~= nil then
            for i, card in ipairs(self.items) do
                local x, y = self:cardCoords(i)
                love.graphics.draw(self.faceUp and card.image or Deck.back, x, y, 0, scale, scale, 0)
            end
        else
            love.graphics.draw(Deck.back, self.x, self.y, 0, scale, scale, 0)
        end
    end
end
