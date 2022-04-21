import gpr5300
import math


class Scene03(gpr5300.System):
<<<<<<< HEAD
=======

>>>>>>> c7358c073067b6c65530b56fb5dafd45e4e8a030
    def begin(self):
        scene = gpr5300.get_scene()
        self.pipeline = scene.get_pipeline(0)
        self.t = 0.0

    def update(self, dt):
        self.t += dt
        self.pipeline.set_float("value", math.sin(self.t))

    def end(self):
        print("End")