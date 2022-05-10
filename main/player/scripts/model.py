import gpr5300
import math


class ModelScene(gpr5300.System):
    def begin(self):
        pass

    def end(self):
        pass

    def update(self, dt: float):
        pass

    def draw(self, subpass_index: int):
        subpass = gpr5300.get_scene().get_subpass(subpass_index)
        draw_command_count = subpass.draw_command_count
        for command_index in range(0, draw_command_count):
            draw_command = subpass.get_draw_command(command_index)
            material = draw_command.get_material()
            material.bind()
            pipeline = material.get_pipeline()
            model = gpr5300.Mat4(1.0)
            model = gpr5300.Mat4.scale(model, gpr5300.Vec3(0.1,0.1,0.1))
            pipeline.set_mat4("model", model)
            pipeline.set_mat4("projection", 
                gpr5300.Mat4.perspective(math.radians(45), gpr5300.get_aspect(), 0.1, 100.0))

            draw_command.draw()
