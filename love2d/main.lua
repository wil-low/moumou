ButtonManager = require('lib/simplebutton/simplebutton')

require("game")

WIDE = 1280
HIGH = 1000

STATE = nil

mainFont = love.graphics.newFont("font/LeagueSpartan-Regular.ttf", 28)

love.window.setTitle(' Moumou ')
love.window.setMode( WIDE,HIGH )

love.graphics.setFont(mainFont)
ButtonManager.default.font = mainFont

function love.load()
    math.randomseed(45) -- os.time())
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
end
