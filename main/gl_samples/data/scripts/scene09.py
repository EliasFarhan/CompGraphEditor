from neko2 import *
import math


class Scene09(System):

    def begin(self):
        scene = get_scene()
        self.camera = scene.camera

    def update(self, dt):
        pass

    def draw(self, draw_command: DrawCommand):
        draw_command.bind()
        if draw_command.name == "model":
            draw_command.set_vec3("cameraPos", self.camera.position)
            draw_command.set_mat4("model", Mat4(1.0))
            draw_command.set_mat4("normalMatrix", Mat4(1.0))
            draw_command.set_mat4("view", self.camera.view)
        elif draw_command.name == "skybox":
            draw_command.set_mat4("view", Mat4(Mat3(self.camera.view)))

        draw_command.set_mat4("projection", Mat4.perspective(math.radians(45), get_aspect(), 0.1, 100.0))
        draw_command.draw()

    def end(self):
        pass
