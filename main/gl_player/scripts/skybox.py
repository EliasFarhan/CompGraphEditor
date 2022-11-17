import gpr5300
import math

class Skybox(gpr5300.System):
    def begin(self):
        scene = gpr5300.get_scene()
        subpass = scene.get_subpass(0)
        self.command = subpass.get_draw_command(0)
        material = self.command.get_material()
        self.pipeline = material.get_pipeline()
    def update(self, dt: float):
        pass
    def end(self):
        pass
    def draw(self, subpass_index: int):
        self.pipeline.set_mat4("projection", gpr5300.Mat4.perspective(math.radians(45.0), gpr5300.get_aspect(), 0.1, 100.0))
        self.command.draw()