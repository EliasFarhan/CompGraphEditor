from enum import Enum
from __future__ import annotations


class Key(Enum):
    A = 1
    B = 2
    C = 3
    D = 4
    E = 5
    F = 6
    G = 7
    H = 8
    I = 9
    J = 10
    K = 11
    L = 12
    M = 13
    N = 14
    O = 15
    P = 16
    Q = 17
    R = 18
    S = 19
    T = 20
    U = 21
    V = 22
    W = 23
    X = 24
    Y = 25
    Z = 26
    BACKSPACE = 27
    DELETE = 28
    UP = 29
    DOWN = 30
    LEFT = 31
    RIGHT = 32
    ESCAPE = 33
    LALT = 34
    RALT = 35
    LCTRL = 36
    RCTRL = 37
    LSHIFT = 38
    RSHIFT = 39
    SPACE = 40
    RETURN = 41

class Vec2:
    def __init__(self):
        self.x = 0.0
        self.y = 0.0
        self.length = 0.0
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __init__(self, v: Vec3) -> None:
        pass
    def __init__(self, v: Vec4) -> None:
        pass

    def __add__(self, other:Vec2) -> Vec2:
        return Vec2()
    
    def __sub__(self, other:Vec2) -> Vec2:
        return Vec2()

    def __mul__(self, other:float) -> Vec2:
        return Vec2()

    def normalize(self) -> Vec2:
        return Vec2()

    @staticmethod
    def dot(v1: Vec2, v2: Vec2) -> float:
        return 0.0


class Vec3:
    def __init__(self):
        self.x = 0.0
        self.y = 0.0
        self.z = 0.0
        self.length = 0.0

    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z
    
    def __init__(self, v: Vec2, f:float) -> None:
        pass
    def __init__(self, v: Vec4) -> None:
        pass

    def __add__(self, other: Vec3) -> Vec3:
        return Vec3()

    def __sub__(self, other: Vec3) -> Vec3:
        return Vec3()

    def __mul__(self, other:float) -> Vec3:
        return Vec3()

    def normalize(self) -> Vec3:
        return Vec3()

    @staticmethod
    def cross(v1: Vec3, v2: Vec3) -> Vec3:
        return Vec3()

    @staticmethod
    def dot(v1: Vec3, v2: Vec3) -> float:
        return 0.0


class Vec4:
    def __init__(self):
        self.x = 0.0
        self.y = 0.0
        self.z = 0.0
        self.w = 0.0
        self.length = 0.0

    def __init__(self, x, y, z, w):
        self.x = x
        self.y = y
        self.z = z
        self.w = w
    
    def __init__(self, v: Vec2, f1: float, f2: float) -> None:
        pass
    def __init__(self, v:Vec3, f:float) -> None:
        pass
    
    def __add__(self, other: Vec4) -> Vec4:
        return Vec4()

    def __sub__(self, other: Vec4) -> Vec4:
        return Vec4()

    def __mul__(self, other:float) -> Vec4:
        return Vec4()


    def normalize(self) -> Vec4:
        return Vec4()

    @staticmethod
    def dot(v1: Vec4, v2: Vec4):
        return 0.0

class Mat4:
    def __init__(self):
        pass

    def translate(self, v: Vec3)-> Mat4:
        return Mat4()
    
    def scale(self, v: Vec3)-> Mat4:
        return Mat4()

    def rotate(self, radian: float, axis: Vec3)-> Mat4:
        return Mat4()

    def __add__(self, other: Mat4) -> Mat4:
        return Mat4()

    def __sub__(self, other: Mat4) -> Mat4:
        return Mat4()

    def __mul__(self, other:float) -> Mat4:
        return Mat4()

    def __mul__(self, other:Vec4) -> Vec4:
        return Vec4()

    def __mul__(self, other: Mat4) -> Mat4:
        return Mat4()

    @staticmethod
    def view(eye:Vec3, center:Vec3, up:Vec3) -> Mat4:
        return Mat4()
    
    @staticmethod
    def perspective(fov_radian: float, aspect: float, near: float, far: float) -> Mat4:
        return Mat4()
    

class Pipeline:
    def __init__(self) -> None:
        pass
    def set_float(self, uniform_name: str, v: float):
        pass
    def set_vec2(self, uniform_name: str, v: Vec2):
        pass
    def set_vec3(self, uniform_name: str, v: Vec3):
        pass
    def set_vec4(self, uniform_name: str, v: Vec4):
        pass
    def set_mat4(self, uniform_name: str, m:Mat4):
        pass
        

class Material:
    def __init__(self) -> None:
        pass
    def bind(self):
        pass
    def get_pipeline(self) -> Pipeline:
        return Pipeline()


class System:
    def __init__(self):
        pass
    def begin(self):
        pass
    def update(self, dt: float):
        pass
    def end(self):
        pass
    def draw(self, subpass_index: int):
        pass
    def on_key_down(self, keycode: Key):
        pass
    def on_key_up(self, keycode: Key):
        pass
    def on_motion_motion(self, motion: Vec2):
        pass

class DrawCommand:
    def __init__(self) -> None:
        pass
    def get_material(self, material_index: int) -> Material:
        return Material()
    def draw(self):
        pass

class SubPass:
    def __init__(self):
        self.draw_command_count = 0
    def get_draw_command(self,draw_command_index: int) -> DrawCommand:
        return DrawCommand()


class Scene:
    def __init__(self) -> None:
        self.subpass_count = 0
        self.pipeline_count = 0
        self.material_count = 0
    def get_pipeline(self,pipeline_index:int) -> Pipeline:
        return Pipeline()
    def get_material(self,material_index: int) -> Material:
        return Material()
    def get_subpass(self,subpass_index: int) -> SubPass:
        return SubPass()

def get_window_size():
    return Vec2()

def get_aspect():
    return 0.0

def get_scene():
    return Scene()

