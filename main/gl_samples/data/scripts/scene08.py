import core
import math


class Scene08(core.System):
    def begin(self):
        pass

    def update(self, dt: float):
        pass

    def draw(self, draw_command: core.DrawCommand):
        if draw_command.subpass_index == 0:
            draw_command.bind()
            draw_command.set_mat4("projection", core.Mat4.perspective(math.radians(45), core.get_aspect(), 0.1, 100.0))
            draw_command.set_mat4("model", core.Mat4(1.0))
            draw_command.draw()

    def end(self):
        pass
