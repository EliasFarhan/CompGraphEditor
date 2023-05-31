from neko2 import *
import math


class Scene08(System):
    def begin(self):
        pass

    def update(self, dt: float):
        pass

    def draw(self, draw_command: DrawCommand):
        camera = get_scene().camera
        if draw_command.subpass_index == 0:
            draw_command.bind()
            draw_command.set_mat4("projection", camera.projection)
            draw_command.set_mat4("view", camera.view)
            draw_command.set_mat4("model", Mat4(1.0))
            draw_command.draw()

    def end(self):
        pass
