import core
import math


class Scene07(core.System):
    def begin(self):
        scene = core.get_scene()
        subpass = scene.get_subpass(0)
        self.draw_command = subpass.get_draw_command(0)
        self.material = scene.get_material(0)
        self.pipeline = self.material.get_pipeline()

    def update(self, dt: float):
        self.material.bind()
        self.pipeline.set_mat4("projection", core.Mat4.perspective(math.radians(45), core.get_aspect(), 0.1, 100.0))
        self.pipeline.set_mat4("model", core.Mat4(1.0))
        self.draw_command.draw()

    def end(self):
        pass
