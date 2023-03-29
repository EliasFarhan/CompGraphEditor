import core
import math


class Camera(core.System):
    def begin(self):
        scene = core.get_scene()
        self.camera = scene.get_camera()
        self.camera.position = core.Vec3(0,0,3)
        self.camera.direction = core.Vec3(0,0,-1)
        self.speed = 1.0
        self.keys = [False, False, False, False]
        self.mouse_motion = core.Vec2()

    def update(self, dt: float):
        scene = core.get_scene()
        self.camera = scene.get_camera()
        direction = core.Vec3()
        if self.keys[0]:
            direction += core.Vec3(1, 0, 0)
        if self.keys[1]:
            direction -= core.Vec3(1, 0, 0)
        if self.keys[2]:
            direction += core.Vec3(0, 1, 0)
        if self.keys[3]:
            direction -= core.Vec3(0, 1, 0)
        horizontal = core.Vec3.cross(self.camera.up, self.camera.direction).normalize()
        vertical = core.Vec3.cross(horizontal, self.camera.direction)
        window_size = core.get_window_size()
        self.mouse_motion.x /= window_size.x
        self.mouse_motion.y /= window_size.y
        rotate_x = core.Mat4.rotate(core.Mat4(1.0), self.mouse_motion.x*math.pi, vertical)
        rotate_y = core.Mat4.rotate(core.Mat4(1.0), self.mouse_motion.y*math.pi, horizontal)
        self.mouse_motion = core.Vec2()
        self.camera.direction = core.Vec3(rotate_x*rotate_y*core.Vec4(self.camera.direction, 1.0))

        self.camera.position += (horizontal*direction.x+self.camera.direction*direction.y)*self.speed*dt
        
    def end(self):
        pass

    def on_key_down(self, keycode: core.Key):
        if keycode == int(core.Key.A):
            self.keys[0] = True
        elif keycode == int(core.Key.D):
            self.keys[1] = True
        elif keycode == int(core.Key.W):
            self.keys[2] = True
        elif keycode == int(core.Key.S):
            self.keys[3] = True

    def on_key_up(self, keycode: core.Key):
        if keycode == int(core.Key.A):
            self.keys[0] = False
        elif keycode == int(core.Key.D):
            self.keys[1] = False
        elif keycode == int(core.Key.W):
            self.keys[2] = False
        elif keycode == int(core.Key.S):
            self.keys[3] = False

    def on_mouse_motion(self, motion: core.Vec2):
        self.mouse_motion = motion
