from neko2 import *
import math


class SubpassSystem(System):

    def begin(self):
        self.t = 0.0

    def update(self, dt):
        self.t += dt

    def draw(self, draw_command: DrawCommand):
        if draw_command.name == "CubeCommand":
            draw_command.bind()
            draw_command.set_mat4("ubo.model", Mat4(1.0))
            draw_command.set_mat4("ubo.view", get_scene().get_camera().view)
            draw_command.set_mat4("ubo.projection", Mat4.perspective(math.radians(45), get_aspect(), 0.1,
                                                                          100.0))
            draw_command.draw()
        if draw_command.name == "SphereCommand":
            draw_command.bind()
            draw_command.set_mat4("ubo.model", Mat4(1.0).translate(Vec3(3,0,0)))
            draw_command.set_mat4("ubo.view", get_scene().get_camera().view)
            draw_command.set_mat4("ubo.projection", Mat4.perspective(math.radians(45), get_aspect(), 0.1,
                                                                          100.0))
            draw_command.draw()

    def end(self):
        print("End")
