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
            material = draw_command.get_material()
            material.bind()
            if draw_command.name == "model":
                pipeline = material.get_pipeline()
                pipeline.set_vec3("cameraPos", self.camera.position)
                pipeline.set_mat4("model", core.Mat4(1.0))
                pipeline.set_mat4("normalMatrix", core.Mat4(1.0))
                
                pipeline.set_mat4("view", self.camera.view)
            elif draw_command.name == "skybox":
                pipeline = material.get_pipeline()
                pipeline.set_mat4("view", core.Mat4(core.Mat3(self.camera.view)))

            pipeline.set_mat4("projection", core.Mat4.perspective(math.radians(45), core.get_aspect(), 0.1, 100.0))
            draw_command.draw()

    def end(self):
        pass