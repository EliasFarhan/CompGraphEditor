import core
import math


class Scene09(core.System):

    def begin(self):
        scene = core.get_scene()
        self.subpass = scene.get_subpass(0)
        self.camera = scene.camera

    def update(self, dt):
        pass

    def draw(self, subpass_index: int):
        for i in range(0, self.subpass.draw_command_count):
            draw_command = self.subpass.get_draw_command(i)
            if draw_command.name == "model":
                pipeline = draw_command.get_material().get_pipeline()
                pipeline.set_vec3("cameraPos", self.camera.position)
                pipeline.set_mat4("model", core.Mat4(1.0))
                pipeline.set_mat4("normalMatrix", core.Mat4(1.0))
                pass
            elif draw_command.name == "skybox":
                pipeline = draw_command.get_material().get_pipeline()

            pipeline.set_vec3("view", self.camera.view)
            pipeline.set_mat4("projection", core.Mat4.perspective(math.radians(45), core.get_aspect(), 0.1, 100.0))


    def end(self):
        pass