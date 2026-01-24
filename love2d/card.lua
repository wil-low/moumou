require("core")

local SUITS = "SHDC"
local VALUES = "6789TJQKA"

Card = {}

function Card.init(suit, value, x, y)
    local self = setmetatable({}, Card)
    self.image = love.graphics.newImage("img/" .. string.sub(VALUES, value, value) .. string.sub(SUITS, suit, suit) .. ".png")
    self.suit = suit
    self.value = value
    self.x = x
    self.y = y
    return self
end

