import core
import math


class Scene07(core.System):
    def begin(self):
        pass

    def update(self, dt: float):
        pass

    def draw(self, draw_command: core.DrawCommand):
        draw_command.bind()
        draw_command.set_mat4("ubo.projection", core.Mat4.perspective(math.radians(45), core.get_aspect(), 0.1, 100.0))
        draw_command.set_mat4("ubo.model", core.Mat4(1.0))
        draw_command.set_mat4("ubo.view", core.get_scene().get_camera().view)
        draw_command.draw()

    def end(self):
        pass
