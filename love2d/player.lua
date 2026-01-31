Player = {}
Player.__index = Player

function Player.init(idx, level, x, y)
    local self = setmetatable({}, Player)
    self.idx = idx
    self.level = level
    self.score = 0
    self.hand = Deck.init("player " .. idx, x + slot_x, y, level == AILevel.Human, slot_x * 7)
    return self
end

function Player:draw()
    love.graphics.setColor(0, 0, 0)
    love.graphics.print("Score: " .. self.score, self.hand.x - slot_x, self.hand.y + pad)
    if self.level == AILevel.Human then
        love.graphics.print("Hand: " .. self:handScore(), self.hand.x - slot_x, self.hand.y + pad * 3)
    else
        love.graphics.print("Cards: " .. #self.hand.items, self.hand.x - slot_x, self.hand.y + pad * 3)
    end
    love.graphics.setColor(1, 1, 1)
    self.hand:draw()
end

function Player:handScore()
    local score = 0
    for _, card in ipairs(self.hand.items) do
        local val = card.value
        if val == Value.Ten or val == Value.Queen or val == Value.King then
            score = score + 10
        elseif val == Value.Ace then
            score = score + 15
        elseif val == Value.Jack then
            score = score + 20
        end
    end
    return score
end

function Player:clearHand()
    self.hand:clear()
end

function Player:inputMove(game)
    if self.level ~= AILevel.Human then
        game:disableButtons()
    end
    if self.level == AILevel.Level_1 then
        if #game.validMoves.items > 0 then
            return game.validMoves.items[1]
        end
        if game.validMoves.pass then
            return Move.Pass
        end
        if game.validMoves.draw then
            return Move.Draw
        end
    elseif self.level == AILevel.Level_2 then
        if #game.validMoves.items > 0 then
            return game.validMoves.items[math.random(1, #game.validMoves.items)]
        end
        if game.validMoves.draw then
            return Move.Draw
        end
        if game.validMoves.pass then
            return Move.Pass
        end
    end
    return nil
end

function Player:inputSuit(game)
    if self.level == AILevel.Level_1 then
        return math.random(1, Suit.Count)
    elseif self.level == AILevel.Level_2 then
        local countBySuit = {0, 0, 0, 0}
        for _, card in ipairs(self.hand.items) do
            countBySuit[card.suit] = countBySuit[card.suit] + 1
        end
        local maxIdx = 1
        for i = 1, Suit.Count do
            if countBySuit[i] > countBySuit[maxIdx] then
                maxIdx = i
            end
        end
        return maxIdx
    else  -- Human
        game.passButton.interactable = false
        for i = 1, Suit.Count do
            game.suitButtons[i].interactable = true
            game.suitButtons[i].enabled = true
        end
    end
end
