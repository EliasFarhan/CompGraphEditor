from neko2 import *
import numpy as np
import math
import random


class UniformInstancingScene(System):
    def begin(self):
        pos = Vec3(1,2,0)
        pos_view = memoryview(pos)
        print("Vec3 memory view: {}".format(pos_view[:]))

        self.pos_buffer = get_buffer("positions")

        self.gravity_const = 1000.0
        self.center_mass = 1000.0
        self.planet_mass = 1.0

        view = self.pos_buffer.memory_view(Vec3)
        print("View: {} Itemsize: {} Nbytes: {}".format(view.format, view.itemsize, view.nbytes))
        #print("View content: {}".format(view[0]))
        for i in range(10000):
            tmp = Vec3(random.randint(10, 50), 0.0, random.randint(10, 50))
            view[i] = memoryview(tmp)[:]

        camera = get_camera_system().camera
        camera.position = Vec3(100, 100, 100)
        camera.direction = -camera.position.normalize()
        camera.far = 500

    def end(self):
        pass

    def update(self, dt: float):
        view = self.pos_buffer.memory_view(Vec3)
        for i in range(10000):
            pos = Vec3()
            memoryview(pos)[:] = view[i]
            delta = -pos
            r = delta.length
            force_value = self.gravity_const * self.center_mass * self.planet_mass / (r * r)
            vel_dir = Vec3(-delta.z, 0.0, delta.x).normalize()

            speed = math.sqrt(force_value / self.planet_mass * r)
            vel = vel_dir * speed

            pos = pos + vel * dt
            view[i] = memoryview(pos)

    def draw(self, draw_command: DrawCommand):
        camera = get_scene().camera
        draw_command.bind()
        draw_command.set_mat4("view", camera.view)
        draw_command.set_mat4("projection", camera.projection)
        draw_command.draw_instanced(10000)
