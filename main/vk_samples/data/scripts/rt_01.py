import math

from neko2 import *

class Raytracing01(System):
    def begin(self):
        pass

    def end(self):
        pass

    def update(self, dt: float):
        pass

    def dispatch(self, command: Command):
        command.set_mat4("cam.viewInverse", Mat4.view(Vec3(0,0,-10), Vec3(0,0,0), Vec3(0,1,0)).inverse())
        command.set_mat4("cam.projInverse", Mat4.perspective(math.radians(45), get_aspect(), 0.1, 100).inverse())
