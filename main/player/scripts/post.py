import gpr5300
import math

class PostProcess(gpr5300.System):
    def begin(self):
        scene = gpr5300.get_scene()
        self.subpass0 = scene.get_subpass(0)
        self.framebuffer = self.subpass0.get_framebuffer()
        
        subpass1 = scene.get_subpass(1)
        self.command = subpass1.get_draw_command(0)
        material = self.command.get_material()
        self.pipeline = material.get_pipeline()
        self.cube_pos = gpr5300.Vec3(1,0,0)
        self.model_pos = gpr5300.Vec3(-1,0,0)

    def end(self):
        pass

    def update(self, dt: float):
        pass

    def draw(self, subpass_index: int):
        if subpass_index == 0:
            for i in range(0, self.subpass0.draw_command_count):
                command = self.subpass0.get_draw_command(i)
                pipeline = command.get_material().get_pipeline()
                pipeline.set_mat4("projection", gpr5300.Mat4.perspective(math.radians(45.0),gpr5300.get_aspect(), 0.1, 100))
                if i == 0:
                    #cube
                    model = gpr5300.Mat4.translate(gpr5300.Mat4(1.0), self.cube_pos)
                    pipeline.set_mat4("model", model)
                else:
                    #model
                    model = gpr5300.Mat4.translate(gpr5300.Mat4(1.0), self.model_pos)
                    model = gpr5300.Mat4.scale(model, gpr5300.Vec3(0.1, 0.1, 0.1))
                    pipeline.set_mat4("model", model)
                command.draw()
        elif subpass_index == 1:
            self.pipeline.set_texture("screenTexture", self.framebuffer.get_texture_name("color_buffer"), 0)
            self.command.draw()
