from neko2 import *
import math


class Scene07(System):
    def begin(self):
        pass

    def update(self, dt: float):
        pass

    def draw(self, draw_command: DrawCommand):
        draw_command.bind()
        draw_command.set_mat4("ubo.projection", Mat4.perspective(math.radians(45), get_aspect(), 0.1, 100.0))
        draw_command.set_mat4("ubo.model", Mat4(1.0))
        draw_command.set_mat4("ubo.view", get_scene().get_camera().view)
        draw_command.draw()

    def end(self):
        pass
