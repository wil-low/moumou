require("game")

WIDE = 1280
HIGH = 1000

STATE = nil

love.window.setTitle(' Moumou ')
love.window.setMode( WIDE,HIGH )

function love.load()
    math.randomseed(45) -- os.time())
    love.graphics.setBackgroundColor(0.3,0.5,0.3)
    game = Game.init()
    game:new()
    x = 50
    y = 50
    speed = 300
    STATE = game
end

function love.update(dt)
    if love.keyboard.isDown("right") then
        x = x + (speed * dt)
    end
    if love.keyboard.isDown("left") then
        x = x - (speed * dt)
    end

    if love.keyboard.isDown("down") then
        y = y + (speed * dt)
    end
    if love.keyboard.isDown("up") then
        y = y - (speed * dt)
    end
end

function love.draw()
    STATE:draw()
end
