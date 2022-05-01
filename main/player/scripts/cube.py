import gpr5300
import math

class Cube(gpr5300.System):
    def begin(self):
        scene = gpr5300.get_scene()
        subpass = scene.get_subpass(0)
        self.draw_command = subpass.get_draw_command(0)
        self.material = scene.get_material(0)
        self.pipeline = self.material.get_pipeline()
        self.t = 0.0
        self.positions = [gpr5300.Vec3(),
            gpr5300.Vec3(2.0, 5.0, -15.0),
            gpr5300.Vec3(-1.5, -2.2, -2.5),
            gpr5300.Vec3(-3.8, -2.0, -12.3),
            gpr5300.Vec3(2.4, -0.4, -3.5),
            gpr5300.Vec3(-1.7, 3.0, -7.5),
            gpr5300.Vec3(1.3, -2.0, -2.5),
            gpr5300.Vec3(1.5, 2.0, -2.5),
            gpr5300.Vec3(1.5, 0.2, -1.5),
            gpr5300.Vec3(-1.3, 1.0, -1.5)]

    def update(self, dt: float):
        self.t += dt

    def end(self):
        pass

    def draw(self, subpassIndex: int):
        self.material.bind()
        self.pipeline.set_mat4("view", gpr5300.Mat4.view(gpr5300.Vec3(0,0,-1), gpr5300.Vec3(0,0,0), gpr5300.Vec3(0,1,0)))
        self.pipeline.set_mat4("projection", gpr5300.Mat4.perspective(math.radians(45), gpr5300.get_aspect(), 0.1, 100.0))
        for pos in self.positions:
            model = gpr5300.Mat4(1.0)
            model = model.rotate(self.t, gpr5300.Vec3(0,0,1))
            model = model.rotate(self.t, gpr5300.Vec3(0,1,0))
            model = model.translate(pos)
            self.pipeline.set_mat4("model", model)
            self.draw_command.draw()
