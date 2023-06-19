from __future__ import annotations
from enum import Enum


class Key(Enum):
    """Key is an enum for keyboard key of SDL"""
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

class Access(Enum):
    """Key is an enum for keyboard key of SDL"""
    READ_ONLY = 1
    WRITE_ONLY = 2
    READ_WRITE = 3

class Vec2:
    """Vec2 is a math struct representing a 2d float vector"""
    length: float
    x: float
    y: float

    def __init__(self):
        """Default constructor"""
        self.x = 0.0
        self.y = 0.0
        self.length = 0.0

    def __init__(self, x: float, y: float):
        """Constructor with 2 floats"""
        self.x = x
        self.y = y

    def __init__(self, v: Vec3):
        """Constructor with a Vec3, discarding z"""
        self.x = v.x
        self.y = v.y

    def __init__(self, v: Vec4):
        """Constructor with a Vec4, discarding z and w"""
        pass

    def __add__(self, other: Vec2) -> Vec2:
        """Vector addition"""
        return Vec2()
    
    def __sub__(self, other: Vec2) -> Vec2:
        """Vector subtraction"""
        return Vec2()

    def __neg__(self):
        return Vec3()

    def __mul__(self, other: float) -> Vec2:
        """Vector multiplication with scalar"""
        return Vec2()

    def __truediv__(self, other: float) -> Vec2:
        """Vector division with scalar"""
        return Vec2()

    def normalize(self) -> Vec2:
        """Return a normalized (length 1)"""
        return Vec2()

    @staticmethod
    def dot(v1: Vec2, v2: Vec2) -> float:
        """Return the dot product between v1 and v2"""
        return 0.0


class Vec3:
    """Vec3 is a math struct representing a 3d float vector"""
    length: float
    x: float
    y: float
    z: float

    def __init__(self):
        """Default constructor"""
        self.x = 0.0
        self.y = 0.0
        self.z = 0.0
        self.length = 0.0

    def __init__(self, x: float, y: float, z: float):
        """Constructor with 3 floats"""
        self.x = x
        self.y = y
        self.z = z
    
    def __init__(self, v: Vec2, f: float):
        """Constructor with a Vec2 for x and y and a float for z"""
        pass

    def __init__(self, v: Vec4) -> None:
        """Constructor with a Vec4, discarding w"""
        pass

    def __add__(self, other: Vec3) -> Vec3:
        """Vector addition"""
        return Vec3()

    def __sub__(self, other: Vec3) -> Vec3:
        """Vector subtraction"""
        return Vec3()

    def __mul__(self, other: float) -> Vec3:
        """Vector multiplication with a scalar float"""
        return Vec3()

    def __truediv__(self, other: float) -> Vec3:
        return Vec3()


    def __neg__(self):
        return Vec3()

    def normalize(self) -> Vec3:
        """Return the normalized vector (length 1)"""
        return Vec3()

    @staticmethod
    def cross(v1: Vec3, v2: Vec3) -> Vec3:
        """Return the cross product between v1 and v2"""
        return Vec3()

    @staticmethod
    def dot(v1: Vec3, v2: Vec3) -> float:
        """Return the dot product between v1 and v2"""
        return 0.0


class Vec4:
    """Vec4 is a math struct representing a 4d float vector"""
    length: float
    """length is a calculated property that gives the length of the vector"""
    x: float
    y: float
    z: float
    w: float

    def __init__(self):
        """Default constructor"""
        self.x = 0.0
        self.y = 0.0
        self.z = 0.0
        self.w = 0.0
        self.length = 0.0

    def __init__(self, x, y, z, w):
        """Constructor with 4 floats"""
        self.x = x
        self.y = y
        self.z = z
        self.w = w
    
    def __init__(self, v: Vec2, z: float, w: float) -> None:
        """Constructor with a Vec2 for x and y, a float for z, a float for w"""
        pass

    def __init__(self, v: Vec3, w: float) -> None:
        """Constructor with a Vec3 for x, y and z, and a float for w"""
        pass
    
    def __add__(self, other: Vec4) -> Vec4:
        """Vector addition"""
        return Vec4()

    def __sub__(self, other: Vec4) -> Vec4:
        """Vector subtraction"""
        return Vec4()

    def __neg__(self):
        return Vec3()

    def __mul__(self, other: float) -> Vec4:
        """Vector multiplication with a scalar float"""
        return Vec4()

    def __truediv__(self, other: float) -> Vec4:
        """Vector multiplication with a scalar float"""
        return Vec4()

    def normalize(self) -> Vec4:
        """Return the normalized vector (length 1)"""
        return Vec4()

    @staticmethod
    def dot(v1: Vec4, v2: Vec4):
        """Calculate the dot product of v1 and v2"""
        return 0.0


class Mat3:
    """Mat3 represents a 3x3 float matrix"""
    def __init__(self):
        pass

    def __init__(self, m: Mat4):
        """Constructor with a Mat4 removing both 4th row and 4th column"""
        pass

    def __add__(self, other: Mat3) -> Mat3:
        """Matrix addition"""
        return Mat3()

    def __sub__(self, other: Mat3) -> Mat3:
        """Matrix subtraction"""
        return Mat3()

    def __mul__(self, other: float) -> Mat3:
        """Matrix multiplication with a scalar float"""
        return Mat3()

    def __mul__(self, other: Vec3) -> Vec3:
        """Matrix vector multiplication"""
        return Vec3()

    def __mul__(self, other: Mat3) -> Mat3:
        """Matrix multiplication with scalar"""
        return Mat3()

    def __truediv__(self, other: float) -> Mat3:
        """Matrix division with scalar"""
        return Mat3()

    def inverse(self) -> Mat3:
        """Return the inverse matrix"""
        return Mat3()
    
    def transpose(self) -> Mat3:
        """Return the transposed matrix"""
        return Mat3()


class Mat4:
    """Mat4 is a math struct representing a 4x4 float matrix"""
    def __init__(self):
        """Default constructor"""
        pass

    def __init__(self, v: float):
        """Constructor taking a float for the diagonal values"""
        pass

    def __init__(self, m: Mat3):
        """Constructor taking a Mat3 and filling the 4th row and 4th column with 0"""
        pass

    def translate(self, t: Vec3) -> Mat4:
        """Return the translate transform matrix with translation t"""
        return Mat4()
    
    def scale(self, s: Vec3) -> Mat4:
        """Return the scale transform matrix with scale s"""
        return Mat4()

    def rotate(self, radian: float, axis: Vec3) -> Mat4:
        """Return the rotation transform matrix around axis with angle radian"""
        return Mat4()

    def __add__(self, other: Mat4) -> Mat4:
        return Mat4()

    def __sub__(self, other: Mat4) -> Mat4:
        return Mat4()

    def __mul__(self, other: float) -> Mat4:
        return Mat4()

    def __mul__(self, other: Vec4) -> Vec4:
        return Vec4()

    def __mul__(self, other: Mat4) -> Mat4:
        return Mat4()

    def __truediv__(self, other: float) -> Mat4:
        return Mat4()

    @staticmethod
    def view(eye: Vec3, center: Vec3, up: Vec3) -> Mat4:
        """Return the view matrix from camera position eye, with target position center and up vector"""
        return Mat4()
    
    @staticmethod
    def perspective(fov_radian: float, aspect: float, near: float, far: float) -> Mat4:
        """Return the perspective matrix from fov (field of view) in radian, aspect ratio, near and far plane scalar value"""
        return Mat4()

    @staticmethod
    def orthographic(width: float, height: float, near: float, far: float) -> Mat4:
        """Return the orthographic matrix from width, height and near and far plane scalar value"""
        return Mat4()
    
    def inverse(self) -> Mat4:
        """Return the inverse matrix"""
        return Mat4()
    
    def transpose(self) -> Mat4:
        """Return the transposed matrix"""
        return Mat4()
    

class Pipeline:
    """Pipeline is an abstraction representing the underlying pipeline (rasterizer, compute or raytracing)"""
    name: str

    def __init__(self) -> None:
        self.name = ""
   
    def get_name(self) -> str:
        return self.name


class Material:
    """Material is a pipeline with textures"""
    name: str

    def __init__(self) -> None:
        self.name = ""

    def get_pipeline(self) -> Pipeline:
        return Pipeline()

    def get_name(self) -> str:
        return self.name


class Framebuffer:
    """Framebuffer is an abstraction of the GL-like framebuffer with several render targets to be drawn on."""
    name: str

    def __init__(self) -> None:
        self.name = ""

    

    def get_name(self) -> str:
        return ""


class Command:
    def set_bool(self, uniform_name: str, v: bool):
        pass

    def set_int(self, uniform_name: str, v: int):
        pass

    def set_float(self, uniform_name: str, v: float):
        """Set a named uniform float value"""
        pass

    def set_vec2(self, uniform_name: str, v: Vec2):
        """Set a named uniform Vec2 value"""
        pass

    def set_vec3(self, uniform_name: str, v: Vec3):
        """Set a named uniform Vec3 value"""
        pass

    def set_vec4(self, uniform_name: str, v: Vec4):
        """Set a named uniform Vec4 value"""
        pass

    def set_mat3(self, uniform_name: str, m: Mat3):
        pass

    def set_mat4(self, uniform_name: str, m: Mat4):
        """Set a named uniform Mat4 value"""
        pass

    def set_image(self, image: Image, binding: int, access: Access):
        pass


class ComputeCommand(Command):
    pass


class ModelTransformMatrix:
    """ModelTransformMatrix is an abstraction of a model transform matrix with a translate, scale and euler rotation.
    It is used by a DrawCommand to place the given mesh in the world space."""
    def __init__(self):
        self.translate = Vec3()
        self.scale = Vec3()
        self.rotation = Vec3()
        self.transform = Mat4()

class DrawCommand:
    """DrawCommand is an abstraction representing a draw call.
    It has a mesh and a material.
    You can set uniforms values from a DrawCommand."""
    def __init__(self) -> None:
        self.name = ""
        self.mesh_name = ""
        self.subpass_index = 0
        self.model_matrix = ModelTransformMatrix()

    def get_material(self) -> Material:
        """Return the Material used in the DrawCommand"""
        return Material()

    def get_name(self) -> str:
        """Return the name of the DrawCommand"""
        return ""

    def get_mesh_name(self) -> str:
        """Return the name of the Mesh used in the DrawCommand"""
        return ""

    def set_int(self, uniform_name: str, v: int):
        pass

    def set_float(self, uniform_name: str, v: float):
        """Set a named uniform float value"""
        pass

    def set_vec2(self, uniform_name: str, v: Vec2):
        """Set a named uniform Vec2 value"""
        pass

    def set_vec3(self, uniform_name: str, v: Vec3):
        """Set a named uniform Vec3 value"""
        pass

    def set_vec4(self, uniform_name: str, v: Vec4):
        """Set a named uniform Vec4 value"""
        pass

    def set_mat3(self, uniform_name: str, m: Mat3):
        pass

    def set_mat4(self, uniform_name: str, m: Mat4):
        """Set a named uniform Mat4 value"""
        pass

    def bind(self):
        """Bind all textures from the material, bind the pipeline and the mesh.
        To be used before setting uniforms and drawing."""
        pass

    def draw(self):
        """Perform the draw call of the DrawCommand.
        Typically the last operation with a DrawCommand."""
        pass

    def draw_instanced(self, instance: int):
        """Perform the draw call of the DrawCommand with given instance count.
        Typically the last operation with a DrawCommand."""
        pass





class SubPass:
    def __init__(self):
        self.draw_command_count = 0

    def get_draw_command(self, draw_command_index: int) -> DrawCommand:
        return DrawCommand()

    def get_framebuffer(self):
        return Framebuffer()


class ProjectionType(Enum):
    """ProjectionType is an enum of the camera projection type"""
    PERSPECTIVE = 0
    ORTHOGRAPHIC = 1
    NONE = 2


class Camera:
    """Camera is an abstraction of orthographic or perspective camera.
    It can generate the view and projection matrices given the internal parameters."""
    def __init__(self) -> None:
        self.position = Vec3()
        self.direction = Vec3()
        self.up = Vec3()
        self.near = 0.0
        self.far = 0.0
        self.fov = 0.0
        self.aspect = 0.0
        self.orthographic_size = 0.0
        self.projection_type = ProjectionType.NONE
        self.view = Mat4()
        self.projection = Mat4()

    def get_view(self) -> Mat4:
        return self.view

    def get_projection(self) -> Mat4:
        return self.projection


class Scene:
    camera: Camera
    """camera is the scene camera (typically controlled by the user inputs)."""

    def __init__(self) -> None:
        self.subpass_count = 0
        self.pipeline_count = 0
        self.material_count = 0
        self.camera = Camera()

    def get_pipeline(self, pipeline_index: int) -> Pipeline:
        return Pipeline()

    def get_material(self, material_index: int) -> Material:
        return Material()

    def get_subpass(self, subpass_index: int) -> SubPass:
        return SubPass()

    def get_camera(self) -> Camera:
        return Camera()



class System:
    def __init__(self):
        pass

    def begin(self):
        pass

    def update(self, dt: float):
        pass

    def end(self):
        pass

    def draw(self, draw_command: DrawCommand):
        pass

    def on_key_down(self, keycode: Key):
        pass

    def on_key_up(self, keycode: Key):
        pass

    def on_motion_motion(self, motion: Vec2):
        pass


class CameraSystem(System):
    """CameraSystem is a System that controls the scene camera with keyboard and mouse."""
    def __init__(self):
        System.__init__(self)
        self.camera = Camera()

class Buffer:
    def array(self, type_name: type) -> list:
        pass

def get_window_size():
    return Vec2()


def get_aspect():
    return 0.0


def get_scene():
    return Scene()


def get_camera_system():
    """Return the CameraSystem is use."""
    return CameraSystem()

def get_buffer(name: str):
    return Buffer()