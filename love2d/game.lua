require("core")
require("deck")
require("player")

PLAYER_COUNT = 2

Game = {}
Game.__index = Game

function Game.init()
    local self = setmetatable({}, Game)
    scale = Game.scaler(WIDE, 1052)
    self.players = {
        Player.init(1, AILevel.Human, pad, slot_y * 4 - pad),
        Player.init(2, AILevel.Level_1, pad, pad)
    }
    self.validMoves = {}
    return self
end

function Game:new()
    self.deck = Deck.init("deck", pad, HIGH / 2 - slot_y - pad, false)
    self.table = Deck.init("table", pad + slot_x, HIGH / 2 - slot_y - pad, true)
    self.played = Deck.init("played", pad + slot_x, HIGH / 2 + pad, true)
    self.deck:createCards()
    self.lastCard = nil
    self.demandedSuit = nil
    self.moumouCounter = 0
    self.curPlayer = 1

    self.validMoves = {
        items = {},
        draw = false,
        pass = false
    }

    self.players[1]:clearHand()
    self.players[2]:clearHand()
    self:dealToPlayer(1, 5)
    self:dealToPlayer(2, 5)

    self:playCard(self.curPlayer, 1)
    local result = self:findValidMoves(self.curPlayer)

    io.write("validMoves (" .. (result and "Y" or "N") .. "): ")
    for i = 1, #self.validMoves.items do
        io.write(self.validMoves.items[i] .. ", ")
    end
    if self.validMoves.draw then
        io.write("draw, ")
    end
    if self.validMoves.pass then
        io.write("pass, ")
    end
    print()
end

function Game.scaler(WIDE, cardHeight)
    slot_x = WIDE / 9
    slot_y = HIGH / 5
    scale = slot_y / cardHeight
    pad = WIDE * 0.02
    return scale
end

function Game:recycleDeck()
end

function Game:deal()
    if #self.deck.items > 0 then
        local idx = math.random(1, #self.deck.items)
        local result = self.deck.items[idx]
        self.deck.items[idx] = self.deck.items[#self.deck.items]
        table.remove(self.deck.items, #self.deck.items)
        return result
    end
    return nil
    --self.recycleDeck()
    --return self.deal()
end

function Game:dealToPlayer(idx, count)
    for i = 1, count do
        local card = self:deal()
        table.insert(self.players[idx].hand.items, card)
    end
end

function Game:findValidMoves(playerIdx)
    local p = self.players[playerIdx]
    if #p.hand.items == 0 then
        return false
    end

    self.validMoves.items = {}

    for i = 1, #p.hand.items do
        local pCard = p.hand.items[i]
        if #self.played.items then
            if self.lastCard.value == Value.Six then
                if pCard.value ~= Value.Jack and
                    pCard.value ~= self.lastCard.value and
                    pCard._suit ~= self.lastCard._suit then
                        goto continue
                end
            elseif pCard.value ~= self.lastCard.value then
                goto continue
            end
        else
            if pCard.value == Value.Jack or pCard.value == Value.Six then
            elseif self.demandedSuit ~= nil then
                if pCard.suit ~= self.demandedSuit then
                    goto continue
                end
            elseif self.lastCard ~= nil and
                       (pCard.value == self.lastCard.value or
                        pCard.suit == self.lastCard.suit) then
            else
                goto continue
            end
        end
        table.insert(self.validMoves.items, i)
        ::continue::
    end
    if self.lastCard.value == Value.Six then
        self.validMoves.draw = true
        self.validMoves.pass = false
    end
    if #self.validMoves.items == 0 and not self.validMoves.draw then
        self.validMoves.pass = true
    end
    return true
end

function Game:playCard(playerIdx, cardIdx)
    local p = self.players[playerIdx]
    local pCard = p.hand.items[cardIdx]
    -- apply effects
    local result = Play.OK
    local next_player = (playerIdx + 1) % PLAYER_COUNT + 1
    if pCard.value == Value.Eight then
        self:dealToPlayer(next_player, 2)
        result = Play.OPPONENT_SKIPS
    end
    if pCard.value == Value.Ace then
        result = Play.OPPONENT_SKIPS
    end
    if pCard.value == Value.Seven then
        self:dealToPlayer(next_player, 1)
    end
    if pCard.suit == Suit.Clubs and pCard.value == Value.King then
        self:dealToPlayer(next_player, 5)
        result = Play.OPPONENT_SKIPS
    end
    if pCard.value == Value.Jack then
        result = Play.DEMAND_SUIT
    end
    -- check moumou
    if self.lastCard ~= nil and pCard.value == self.lastCard.value then
        self.moumouCounter = self.moumouCounter + 1
        if self.moumouCounter == Suit.Count then
            result = Play.MOUMOU
        end
    else
        self.moumouCounter = 1
    end

    self.validMoves.draw = pCard.value == Six or result == Play.OPPONENT_SKIPS
    self.validMoves.pass = pCard.value ~= Value.Six

    self.lastCard = pCard
    table.insert(self.played.items, pCard)
    table.remove(p.hand.items, cardIdx)

    return result
end

function Game:draw()
    self.players[2]:draw(nil)
    self.deck:draw()
    self.table:draw()
    self.played:draw()
    if self.curPlayer == 1 then
        self.players[1]:draw(self.validMoves)
        if self.validMoves.draw then
        end    
    else
        self.players[1]:draw(nil)
    end
end
