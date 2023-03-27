import core
import math


class UniformSystem(core.System):

    def begin(self):
        self.t = 0.0
        self.positions = [core.Vec3(),
            core.Vec3(2.0, 5.0, -15.0),
            core.Vec3(-1.5, -2.2, -2.5),
            core.Vec3(-3.8, -2.0, -12.3),
            core.Vec3(2.4, -0.4, -3.5),
            core.Vec3(-1.7, 3.0, -7.5),
            core.Vec3(1.3, -2.0, -2.5),
            core.Vec3(1.5, 2.0, -2.5),
            core.Vec3(1.5, 0.2, -1.5),
            core.Vec3(-1.3, 1.0, -1.5)]

    def update(self, dt: float):
        self.t += dt

    def end(self):
        pass

    def draw(self, draw_command):
        draw_command.bind()
        view = core.Mat4(1.0)
        view = core.Mat4.translate(view, core.Vec3(0, 0, -5))
        draw_command.set_mat4("view", view)
        draw_command.set_mat4("projection", core.Mat4.perspective(math.radians(45), core.get_aspect(), 0.1, 100.0))
        for pos in self.positions:
            model = core.Mat4(1.0)
            model = model.translate(pos)
            model = model.rotate(self.t, core.Vec3(0, 0, 1))
            model = model.rotate(self.t, core.Vec3(0, 1, 0))
            draw_command.set_mat4("model", model)
            draw_command.draw()
