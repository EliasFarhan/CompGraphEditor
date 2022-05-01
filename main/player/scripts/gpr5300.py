
class Vec2:
    def __init__(self):
        self.x = 0.0
        self.y = 0.0
        self.length = 0.0
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __add__(self, other:Vec2):
        return Vec2()
    
    def __sub__(self, other:Vec2):
        return Vec2()

    def __mul__(self, other:float):
        return Vec2()

    @staticmethod
    def dot(v1: Vec2, v2: Vec2):
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

    def __add__(self, other:Vec3):
        return Vec3()

    def __sub__(self, other:Vec3):
        return Vec3()

    def __mul__(self, other:float):
        return Vec3()

    @staticmethod
    def cross(v1: Vec3, v2: Vec3):
        return Vec3()

    @staticmethod
    def dot(v1: Vec3, v2: Vec3):
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
    
    def __add__(self, other):
        return Vec4()

    def __sub__(self, other:Vec4):
        return Vec4()

    def __mul__(self, other:float):
        return Vec4()


    @staticmethod
    def dot(v1: Vec4, v2: Vec4):
        return 0.0

class Mat4:
    def __init__(self):
        pass

    def translate(self, v: Vec3):
        return Mat4()
    
    def scale(self, v: Vec3):
        return Mat4()

    def rotate(self, radian: float, axis: Vec3):
        return Mat4()

    def __add__(self, other:Mat4):
        return Mat4()

    def __sub__(self, other:Mat4):
        return Mat4()

    def __mul__(self, other:float):
        return Mat4()

    def __mul__(self, other:Vec4):
        return Vec4()

    def __mul__(self, other:Mat4):
        return Mat4()

    @staticmethod
    def view(eye:Vec3, center:Vec3, up:Vec3):
        return Mat4()
    
    @staticmethod
    def perspective(fov_radian: float, aspect: float, near: float, far: float):
        return Mat4()
    

class Pipeline:
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
    def __init__(self, pipeline:Pipeline):
        self.pipeline = pipeline
    def bind(self):
        pass
    def get_pipeline(self):
        return self.pipeline

class System:
    def __init__(self):
        pass
    def begin(self):
        pass
    def update(self, dt: float):
        pass
    def end(self):
        pass
    def draw(subpassIndex: int):
        pass

class Scene:
    def get_pipeline(pipeline_index:int):
        pass
    def get_material(material_index: int):
        pass

def get_window_size():
    return Vec2i()

def get_aspect():
    return 0.0

def get_scene():
    return Scene()

