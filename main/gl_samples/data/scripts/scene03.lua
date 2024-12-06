Scene03 = System:new()

function Scene03:begin ()
    self.t = 0.0
end

function Scene03:update(dt)
    self.t += dt
end

function Scene03:draw(draw_command)
    draw_command.bind()
    draw_command.set_float("value", (math.sin(self.t)+1.0)/2.0)
    draw_command.draw()
end

function Scene03:end()
end