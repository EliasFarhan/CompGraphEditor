import core
import math


class UniformSystem(core.System):

    def begin(self):
        self.t = 0.0

    def update(self, dt):
        self.t += dt

    def draw(self, draw_command: core.DrawCommand):
        draw_command.bind()
        draw_command.set_float("constant_values.value", (math.sin(self.t)+1)/2)
        draw_command.draw()

    def end(self):
        print("End")
