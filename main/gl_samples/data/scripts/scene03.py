import core
import math


class Scene03(core.System):

    def begin(self):
        scene = core.get_scene()
        self.pipeline = scene.get_pipeline(0)
        self.t = 0.0

    def update(self, dt):
        self.t += dt
        self.pipeline.set_float("value", (math.sin(self.t)+1)/2)

    def end(self):
        print("End")