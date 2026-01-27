require("core")
require("deck")
require("gametable")
require("player")
require('lib/simplebutton/simplebutton')

STARTING_HAND = 5

Game = {}
Game.__index = Game

function Game.init()
    local self = setmetatable({}, Game)
    love.resize(love.graphics.getWidth(), love.graphics.getHeight())
    self.players = {
        Player.init(1, AILevel.Human, pad * 3, slot_y * 4 - pad),
        Player.init(2, AILevel.Level_1, pad * 3, pad)
    }
    self.pendingMove = nil

    self.deck = Deck.init("deck", pad, love.graphics.getHeight() / 2 - slot_y - pad, false)
    self.table = GameTable.init("table", pad * 3 + slot_x, love.graphics.getHeight() / 2 - slot_y - pad, slot_x * 7)
    self.played = Deck.init("played", pad * 3 + slot_x, love.graphics.getHeight() / 2 + pad, true, slot_x * 7)

    self.drawButton = ButtonManager.new("Draw", self.deck.x, self.deck.y + slot_y + pad,
        Deck.back:getWidth() * scale, Deck.back:getHeight() * scale / 6, false, {0, 1, 0, 1})
    --self.drawButton.enabled = false
    self.drawButton:setAlignment('center')
    self.drawButton.onClick = function()
        self.pendingMove = Move.Draw
    end

    self.passButton = ButtonManager.new("Pass", self.deck.x, self.deck.y + slot_y + pad * 3,
        Deck.back:getWidth() * scale, Deck.back:getHeight() * scale / 6, false, {0, 1, 0, 1})
    --self.passButton.enabled = false
    self.passButton:setAlignment('center')
    self.passButton.onClick = function()
        self.pendingMove = Move.Pass
    end

    self.gameOverButton = ButtonManager.new("", love.graphics.getWidth() / 2 - slot_x - pad, love.graphics.getHeight() / 2 - pad * 2,
        slot_x * 2 + pad * 2, pad * 4, false, {0, 1, 0, 1})
    self.gameOverButton.enabled = false
    self.gameOverButton.onClick = function()
        self.gameOverButton.enabled = false
        self:new()
    end

    local suitImage = love.graphics.newImage("img/C.png")
    self.suitButtons = {}
    local w = suitImage:getWidth() * scale
    local h = suitImage:getHeight() * scale
    for i = 1, Suit.Count do
        local b = ButtonManager.new("", self.deck.x + (w + pad / 2) * ((i - 1) % 2),
                                        self.deck.y + slot_y + pad * 5 + (h + pad / 2) * math.floor((i - 1) / 2),
                                        w, h)
        b:setImage("img/" .. string.sub(Card.SUITS, i, i) .. ".png")
        b.suit = i
        self.suitButtons[i] = b
        b.disabledColor = { 0.8, 0.8, 0.8, 0.8 }
        b.onClick = function()
            self:setDemandedSuit(i)
            self:changePlayer()
            self:turnLoop()
        end
    end
    self.cardButtons = {}

    return self
end

function Game:new()
    self.deck:clear()
    self.table:clear()
    self.played:clear()

    self.deck:createCards()
    self.lastCard = nil
    self.moumouCounter = 0
    self.curPlayer = 1
    self:setDemandedSuit(nil)

    self.validMoves = {
        items = {},
        draw = true,
        pass = false,
        restrictValue = false
    }

    self.players[1]:clearHand()
    self.players[2]:clearHand()

    -- self:dealCard(1, Value.King, Suit.Clubs)
    -- self:dealCard(1, Value.Eight, Suit.Clubs)
    -- self:dealCard(1, Value.Ten, Suit.Hearts)
    -- self:dealCard(1, Value.Queen, Suit.Clubs)
    -- self:dealCard(1, Value.Seven, Suit.Clubs)

    self:dealToPlayer(1, STARTING_HAND)
    self:dealToPlayer(2, STARTING_HAND)

    local card = self:deal()
    table.insert(self.table.items, card)
    self.lastCard = card;

    self:turnLoop()
end

function Game:update(dt)
    if self.pendingMove then
        --print("pendingMove=" .. self.pendingMove .. ", buttons=" .. #ButtonManager.Buttons)
        self:processMove(self.pendingMove)
        self.pendingMove = nil
    end
end

function Game:setDemandedSuit(suit)
    for i = 1, Suit.Count do
        self.suitButtons[i].interactable = false
        self.suitButtons[i].enabled = (i == suit)
    end
    self.demandedSuit = suit
    --print("demandedSuit=" .. tostring(suit))
end

function Game:recycleDeck()
    local n = #self.table.items - 4
    for i = 1, n do
        table.insert(self.deck.items, self.table.items[1])
        table.remove(self.table.items, 1)
    end
end

function Game:deal()
    -- print("Game:deal:", #self.deck.items, #self.table.items)
    if #self.deck.items > 0 then
        local idx = math.random(1, #self.deck.items)
        local result = self.deck.items[idx]
        self.deck.items[idx] = self.deck.items[#self.deck.items]
        table.remove(self.deck.items, #self.deck.items)
        return result
    end
    self:recycleDeck()
    return self:deal()
end

function Game:dealToPlayer(idx, count)
    for i = 1, count do
        local card = self:deal()
        table.insert(self.players[idx].hand.items, card)
    end
end

function Game:dealCard(playerIdx, value, suit)
    local card = Card.init(suit, value, 0, 0)
    table.insert(self.players[playerIdx].hand.items, card)
end

function Game:findValidMoves(playerIdx)
    local p = self.players[playerIdx]
    if #p.hand.items == 0 then
        return false
    end

    self.validMoves.items = {}
    -- print("Game:findValidMoves: lastCard", self.lastCard, "restrictValue", self.validMoves.restrictValue)

    for i = 1, #p.hand.items do
        local pCard = p.hand.items[i]
        local allowed = true
        if self.validMoves.restrictValue and pCard.value ~= self.lastCard.value then
            allowed = false
        elseif pCard.value ~= Value.Jack and pCard.value ~= Value.Six then
            if pCard.value ~= self.lastCard.value then
                if self.demandedSuit ~= nil then
                    allowed = pCard.suit == self.demandedSuit
                else
                    allowed = pCard.suit == self.lastCard.suit
                end
            end
        end
        if allowed then
            table.insert(self.validMoves.items, i)
            if p.level == AILevel.Human then
                local x, y, w, h = p.hand:cardCoords(i)
                local b = self.cardButtons[i]
                if b ~= nil then
                    b.enabled = true
                    b.interactable = true
                    b.x = x
                    b.y = y
                    b.width = w
                    b.height = h
                else
                    b = ButtonManager.new("", x, y, w, h,
                        false, {0, 1, 0, 0.3}, {0, 0.8, 0, 0.3})
                    b.cardIdx = i
                    self.cardButtons[i] = b
                    --print("new button (" .. tostring(b) .. ") = " .. b.cardIdx .. ", total=" .. #ButtonManager.Buttons)
                    b.onClick = function()
                        --print("onClick (" .. tostring(b) .. ") = " .. b.cardIdx)
                        self:disableButtons()
                        self.pendingMove = i
                    end
                end
            end
        end
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

function Game:printValidMoves()
    io.write("validMoves (" .. (result and "Y" or "N") .. "): ")
    for i, v in ipairs(self.validMoves.items) do
        io.write(v .. " ")
    end
    if self.validMoves.draw then
        io.write("draw ")
    end
    if self.validMoves.pass then
        io.write("pass ")
    end
    print()
end

function Game:turnLoop()
    --print("\nturnLoop (player " .. self.curPlayer .. "): ")
    local hasCards = self:findValidMoves(self.curPlayer)

    if not hasCards then
        self:over("has no cards")
        return;
    end
    self.drawButton.interactable = self.validMoves.draw
    self.passButton.interactable = self.validMoves.pass
    --self:printValidMoves()
    self:processMove(self.players[self.curPlayer]:inputMove(self))
    return true
end

function Game:processMove(move)
    --print("processMove (player " .. self.curPlayer .. "): " .. tostring(move))
    if move == nil then
        return  -- human
    end
    if move == Move.Draw then
        self:dealToPlayer(self.curPlayer, 1)
        self.validMoves.draw = false
        print("Player #" .. self.curPlayer .. " draws");
    elseif move == Move.Pass then
        print("Player #" .. self.curPlayer .. " passes");
    else
        local result = self:playCard(self.curPlayer, move)
        if result == Play.Moumou then
            self:over("declares Moumou")
            return
        else
            self:setDemandedSuit(nil)
            self.validMoves.restrictValue = result ~= Play.OpponentSkips and self.lastCard.value ~= Value.Six
        end
    end
    self.validMoves.pass = self.lastCard.value ~= Value.Six

    if move == Move.Pass then
        if #self.played.items > 0 and self.lastCard.value == Value.Jack then 
            local suit = self.players[self.curPlayer]:inputSuit(self)
            if suit ~= nil then
                print("Player #" .. self.curPlayer .. " demands " .. string.sub(Card.SUITS, suit, suit));
                self:setDemandedSuit(suit)
            else
                return false
            end
        end
        self:changePlayer()
    end
    return self:turnLoop()
end

function Game:playCard(playerIdx, cardIdx)
    --print("playCard (" .. playerIdx .. "): " .. cardIdx)
    local p = self.players[playerIdx]
    local pCard = p.hand.items[cardIdx]
    -- apply effects
    local result = Play.OK
    local next_player = playerIdx == 1 and 2 or 1
    if pCard.value == Value.Eight then
        self:dealToPlayer(next_player, 2)
        result = Play.OpponentSkips
    end
    if pCard.value == Value.Ace then
        result = Play.OpponentSkips
    end
    if pCard.value == Value.Seven then
        self:dealToPlayer(next_player, 1)
    end
    if pCard.suit == Suit.Clubs and pCard.value == Value.King then
        self:dealToPlayer(next_player, 5)
        result = Play.OpponentSkips
    end
    -- check moumou
    if self.lastCard ~= nil and pCard.value == self.lastCard.value then
        self.moumouCounter = self.moumouCounter + 1
        if self.moumouCounter == Suit.Count then
            result = Play.Moumou
        end
    else
        self.moumouCounter = 1
    end

    self.validMoves.draw = pCard.value == Six or result == Play.OpponentSkips
    self.validMoves.pass = pCard.value ~= Value.Six

    self.lastCard = pCard
    table.insert(self.played.items, pCard)
    table.remove(p.hand.items, cardIdx)

    print("Player #" .. self.curPlayer .. " plays " .. tostring(self.lastCard))
    return result
end

function Game:changePlayer()
    self:movePlayedToTable()
    self.validMoves.pass = false
    self.validMoves.restrictValue = false
    self.curPlayer = self.curPlayer == 1 and 2 or 1
    print("============== Player changed to " .. self.curPlayer .. " ==============")
end

function Game:movePlayedToTable()
    for _, card in ipairs(self.played.items) do
        table.insert(self.table.items, card)
    end
    self.played:clear()
    self.validMoves.draw = true
end

function Game:disableButtons()
    for _, b in ipairs(ButtonManager.Buttons) do
        if b.cardIdx ~= nil then
            b.enabled = false
        end
    end
end

function Game:updateScores()
    for i = 1, 2 do
        local p = self.players[i]
        p.score = p.score + p:handScore()
    end
end

function Game:over(message)
    self:movePlayedToTable()
    self:updateScores();
    self.gameOverButton:setLabel("Game over!\nPlayer " .. self.curPlayer .. " " .. message, 'center')
    self:disableButtons()
    self.drawButton.interactable = false
    self.passButton.interactable = false
    self.gameOverButton.enabled = true
end

function Game:draw()
    self.players[2]:draw()
    self.deck:draw()
    self.table:draw()
    self.played:draw()
    self.players[1]:draw()
end
