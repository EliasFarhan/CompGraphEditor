import core
import math


class Scene09(core.System):

    def begin(self):
        pass


    def update(self, dt):
        pass

    def draw(self, draw_command: core.DrawCommand):

        draw_command.bind()
        if draw_command.name == "model":
            draw_command.set_vec3("cameraPos", self.camera.position)
            draw_command.set_mat4("model", core.Mat4(1.0))
            draw_command.set_mat4("normalMatrix", core.Mat4(1.0))

            draw_command.set_mat4("view", self.camera.view)
        elif draw_command.name == "skybox":
            draw_command.set_mat4("view", core.Mat4(core.Mat3(self.camera.view)))

        draw_command.set_mat4("projection", core.Mat4.perspective(math.radians(45), core.get_aspect(), 0.1, 100.0))
        draw_command.draw()

    def end(self):
        pass