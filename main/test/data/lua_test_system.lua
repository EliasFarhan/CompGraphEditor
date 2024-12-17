LuaTestSystem = {}
LuaTestSystem.__index = LuaTestSystem

setmetatable(LuaTestSystem, {__index = neko2.System})

function LuaTestSystem.new()
    local self = setmetatable({}, LuaTestSystem)
    neko2.System.new(self)
    return self
end

function LuaTestSystem:on_begin()
    self.t = 0.0
end

function LuaTestSystem:on_update(dt)
    self.t = self.t + dt
end


function LuaTestSystem:on_end()
    self.t = 0.2
end