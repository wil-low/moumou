require("core")
require("deck")
require("gametable")
require("player")
require('lib/simplebutton/simplebutton')

STARTING_HAND = 2

Game = {}
Game.__index = Game

function Game.init()
    local self = setmetatable({}, Game)
    scale = Game.scaler(WIDE, 1052)
    self.players = {
        Player.init(1, AILevel.Human, pad * 3, slot_y * 4 - pad),
        Player.init(2, AILevel.Level_1, pad * 3, pad)
    }
    self.pendingMove = nil

    self.deck = Deck.init("deck", pad, HIGH / 2 - slot_y - pad, false)
    self.table = GameTable.init("table", pad * 3 + slot_x, HIGH / 2 - slot_y - pad)
    self.played = Deck.init("played", pad * 3 + slot_x, HIGH / 2 + pad, true)

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
        draw = false,
        pass = false
    }

    self.players[1]:clearHand()
    self.players[2]:clearHand()
    self:dealToPlayer(1, STARTING_HAND)
    self:dealToPlayer(2, STARTING_HAND)

    self:playCard(self.curPlayer, 1)
    self:turnLoop()
end

function Game.scaler(WIDE, cardHeight)
    slot_x = WIDE / 9
    slot_y = HIGH / 5
    scale = slot_y / cardHeight
    pad = WIDE * 0.02
    return scale
end

function Game:update(dt)
    if self.pendingMove then
        print("pendingMove=" .. self.pendingMove .. ", buttons=" .. #ButtonManager.Buttons)
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
    print("demandedSuit=" .. tostring(suit))
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
        if #self.played.items > 0 then
            if self.lastCard.value == Value.Six then
                if pCard.value ~= Value.Jack and
                    pCard.value ~= self.lastCard.value and
                    pCard.suit ~= self.lastCard.suit then
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
        if p.level == AILevel.Human then
            local x, y = p.hand:cardCoords(i)
            local b = self.cardButtons[i]
            if b ~= nil then
                b.enabled = true
                b.interactable = true
                b.x = x
                b.y = y
            else
                b = ButtonManager.new("", x, y, Deck.back:getWidth() * scale, Deck.back:getHeight() * scale,
                    false, {0, 1, 0, 0.3}, {0, 0.8, 0, 0.3})
                b.cardIdx = i
                print("new button (" .. tostring(b) .. ") = " .. b.cardIdx .. ", total=" .. #ButtonManager.Buttons)
                b.onClick = function()
                    print("onClick (" .. tostring(b) .. ") = " .. b.cardIdx)
                    self:disableButtons()
                    self.pendingMove = i
                end
            end
        end
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

function Game:turnLoop()
    print("\nturnLoop (player " .. self.curPlayer .. "): ")

    local hasCards = self:findValidMoves(self.curPlayer)

    if not hasCards then
        print("Player " .. self.curPlayer .. " has no cards")
        self:updateScores();
        self:new();
        return;
    end
    self.drawButton.interactable = self.validMoves.draw
    self.passButton.interactable = self.validMoves.pass

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

    self:processMove(self.players[self.curPlayer]:inputMove(self))
    return true
end

function Game:processMove(move)
    print("processMove (player " .. self.curPlayer .. "): " .. tostring(move))
    if move == nil then
        return  -- human
    end
    if move == Move.Draw then
        self:dealToPlayer(self.curPlayer, 1)
        self.validMoves.draw = false
    elseif move == Move.Pass then
    else
        local result = self:playCard(self.curPlayer, move)
        self:setDemandedSuit(nil)
        if result == Play.DemandSuit then
            local suit = self.players[self.curPlayer]:inputSuit(self)
            if suit ~= nil then
                self:setDemandedSuit(suit)
            end
        elseif result == Play.OpponentSkips then
            self:movePlayedToTable()
        elseif result == Play.Moumou then
            printf("Player #%d declares Moumou", self.cur_player)
            self:calculate_scores()
            self:new()
            return
        end
    end
    self.validMoves.pass = self.lastCard.value ~= Value.Six

    if move == Move.Pass then
        self:movePlayedToTable()
        self.validMoves.pass = false
        --self.turn++;
        self.curPlayer = self.curPlayer == 1 and 2 or 1
        print("============== Player changed to " .. self.curPlayer .. " ==============")
    end
    return self:turnLoop()   
end

function Game:playCard(playerIdx, cardIdx)
    print("playCard (" .. playerIdx .. "): " .. cardIdx)
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
    if pCard.value == Value.Jack then
        result = Play.DemandSuit
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

    return result
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

function Game:draw()
    self.players[2]:draw()
    self.deck:draw()
    self.table:draw()
    self.played:draw()
    self.players[1]:draw()
end
