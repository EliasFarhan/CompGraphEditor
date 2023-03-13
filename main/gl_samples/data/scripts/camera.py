import core
import math


class Camera(core.System):
    def begin(self):
        self.camera_pos = core.Vec3(0,0,3)
        self.camera_direction = core.Vec3(0,0,-1)
        self.speed = 1.0
        self.keys = [False, False, False, False]
        self.mouse_motion = core.Vec2()

    def update(self, dt: float):
        direction = core.Vec3()
        if self.keys[0]:
            direction += core.Vec3(1, 0, 0)
        if self.keys[1]:
            direction -= core.Vec3(1, 0, 0)
        if self.keys[2]:
            direction += core.Vec3(0, 1, 0)
        if self.keys[3]:
            direction -= core.Vec3(0, 1, 0)
        horizontal = core.Vec3.cross(core.Vec3(0, 1, 0), self.camera_direction).normalize()
        vertical = core.Vec3.cross(horizontal, self.camera_direction)
        window_size = core.get_window_size()
        self.mouse_motion.x /= window_size.x
        self.mouse_motion.y /= window_size.y
        rotate_x = core.Mat4.rotate(core.Mat4(1.0), self.mouse_motion.x*math.pi, vertical)
        rotate_y = core.Mat4.rotate(core.Mat4(1.0), self.mouse_motion.y*math.pi, horizontal)
        self.mouse_motion = core.Vec2()
        self.camera_direction = core.Vec3(rotate_x*rotate_y*core.Vec4(self.camera_direction, 1.0))

        self.camera_pos += (horizontal*direction.x+self.camera_direction*direction.y)*self.speed*dt

    def draw(self, subpass_index: int):
        scene = core.get_scene()
        subpass = scene.get_subpass(subpass_index)
        for i in range(subpass.draw_command_count):
            draw_command = subpass.get_draw_command(i)
            material = draw_command.get_material()
            material.bind()
            pipeline = material.get_pipeline()
            pipeline.set_mat4("view", core.Mat4.view(
                self.camera_pos, 
                self.camera_pos+self.camera_direction, 
                core.Vec3(0, 1, 0)))
    
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
