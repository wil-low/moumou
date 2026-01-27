ButtonManager = require('lib/simplebutton/simplebutton')
require("conf")
require("game")

STATE = nil

mainFont = love.graphics.newFont("font/LeagueSpartan-Regular.ttf", 28)

love.graphics.setFont(mainFont)
ButtonManager.default.font = mainFont

function love.load()
    local seed = os.time()
    --seed = 1769363388
    math.randomseed(seed)
    print("Seed = " .. seed)
    love.graphics.setBackgroundColor(0.3,0.5,0.3)
    game = Game.init()
    game:new()
    STATE = game
end

function love.update(dt)
    STATE:update(dt)
    ButtonManager.update(dt)
end

function love.mousepressed(x, y, msbutton, istouch, presses)
   ButtonManager.mousepressed(x, y, msbutton)
end

function love.mousereleased(x, y, msbutton, istouch, presses)
   ButtonManager.mousereleased(x, y, msbutton)
end

function love.draw()
    STATE:draw()
    ButtonManager.draw()
    --print(love.graphics.getStats().drawcalls)
end

function love.resize(w, h)
    print(("Window resized to width: %d and height: %d."):format(w, h))
    slot_x = w / 9
    slot_y = h / 5
    scale = slot_y / 1052
    pad = w * 0.02
end
