import platform
import subprocess
import os
import json
import re
import pathlib
from enum import Enum

if platform.system() == 'Windows':
    vulkan_path = os.getenv("VULKAN_SDK")
    program = '{}\\Bin\\glslangValidator.exe'.format(vulkan_path)
    compiler = '{}\\Bin\\glslc.exe'.format(vulkan_path)
    spirv_cross = '{}\\Bin\\spirv-cross.exe'.format(vulkan_path)
else:
    program = 'glslangValidator'
    compiler = 'glslc'
    spirv_cross = 'spirv-cross'





def analyze_shader(shader_path):
    """Analyze Vulkan Shader"""
    meta_content = {}
    status = subprocess.run([compiler, shader_path, "-o", shader_path+".spv", '--target-env=vulkan1.0'], capture_output=True, text=True)
    meta_content["stdout"] = status.stdout
    meta_content["stderr"] = status.stderr
    meta_content["returncode"] = status.returncode
    if status.returncode != 0:
        return json.dumps(meta_content)
    status = subprocess.run([spirv_cross, shader_path+".spv", "--output", shader_path+".json", "--reflect"])
    #execute the vulkan shader analyzer
    meta_content["stderr"] = status.stderr
    meta_content["stdout"] = status.stdout
    meta_content["returncode"] = status.returncode
    if status.returncode != 0:
        return json.dumps(meta_content)
    json_output = json.dumps(meta_content)
    return json_output

def main():
    print(analyze_shader("../../gl_samples/data/shaders/scene10/model_instancing.vert"))
    print(analyze_shader("../../gl_samples/data/shaders/scene11/model_instancing.vert"))
    print(analyze_shader("../../gl_samples/data/shaders/scene11/model_uniform_block.vert"))
    #print(analyze_gl_shader("../../gl_samples/data/shaders/scene06/rotated_cube_ubo.vert"))
    #print(analyze_gl_shader("../../gl_samples/data/shaders/scene06/rotated_cube_ubo_block.vert"))
    #print(analyze_gl_shader("../../gl_samples/data/shaders/scene09/skybox.frag"))
    #print(analyze_gl_shader("../shaders/pre_compute_brdf.comp"))


if __name__ == '__main__':
    main()
