Player = {}
Player.__index = Player

function Player.init(idx, ai_level, x, y)
    local self = setmetatable({}, Player)
    self.idx = idx
    self.ai_level = ai_level
    self.score = 0
    self.hand = Deck.init("player " .. idx, x + slot_x, y, true) -- ai_level == AILevel.Human)
    return self
end

function Player:draw(validMoves)
    self.hand:draw()
    if validMoves ~= nil then
        local idx = 1
        love.graphics.setColor(0, 1, 0)
        for i = 1, #self.hand.items do
            if idx <= #validMoves.items and i == validMoves.items[idx] then
                love.graphics.circle("fill", self.hand.x + i * slot_x - pad, self.hand.y + pad - 10, 10)
                idx = idx + 1
            end
        end
        love.graphics.setColor(0, 0, 0)
        love.graphics.rectangle("line", self.hand.x + #self.hand.items * slot_x, self.hand.y, slot_x, slot_y)
    end
end

function Player:clearHand()
    self.hand:clear()
end
