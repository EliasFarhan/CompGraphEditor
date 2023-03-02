import gpr5300
import math


class Scene03(gpr5300.System):

    def begin(self):
        scene = gpr5300.get_scene()
        self.pipeline = scene.get_pipeline(0)
        self.t = 0.0

    def update(self, dt):
        self.t += dt
        self.pipeline.set_float("value", (math.sin(self.t)+1)/2)

    def end(self):
        print("End")