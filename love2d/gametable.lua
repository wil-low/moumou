GameTable = {}
GameTable.__index = GameTable

setmetatable(GameTable, { __index = Deck })

function GameTable.init(label, x, y)
    local self = Deck.init(label, x, y, true)
    return setmetatable(self, GameTable)
end

function GameTable:draw()
    -- draw last 4 cards only
    local n = #self.items
    local start = math.max(1, n - 3)
    for i = start, n do
        local card = self.items[i]
        local x, y = self:cardCoords(i - start + 1)
        love.graphics.draw(card.image, x, y, 0, scale, scale, 0)
    end
end
