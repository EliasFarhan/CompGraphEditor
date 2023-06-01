from neko2 import *
import math
import random


class UniformInstancingScene(System):
    def begin(self):
        self.pos = [Vec3() for i in range(1000)]
        self.gravity_const = 1000.0
        self.center_mass = 1000.0
        self.planet_mass = 1.0

        for i in range(1000):
            self.pos[i] = Vec3(random.randint(10,50), 0.0, random.randint(10,50))

        camera = get_camera_system().camera
        camera.position = Vec3(100, 100, 100)
        camera.direction = -camera.position.normalize()
        camera.far = 500

    def end(self):
        pass

    def update(self, dt: float):
        for i in range(1000):
            pos = self.pos[i]
            delta = -pos
            r = delta.length
            force_value = self.gravity_const * self.center_mass * self.planet_mass / (r * r)
            vel_dir = Vec3(-delta.z, 0.0, delta.x).normalize()

            speed = math.sqrt(force_value / self.planet_mass * r)
            vel = vel_dir * speed

            self.pos[i] = pos + vel * dt

    def draw(self, draw_command: DrawCommand):
        camera = get_scene().camera
        draw_command.bind()
        draw_command.set_mat4("ubo.view", camera.view)
        draw_command.set_mat4("ubo.projection", camera.projection)
        for i in range(1000):
            draw_command.set_vec3("ubo.pos[{}]".format(i), self.pos[i])
        draw_command.draw_instanced(1000)
