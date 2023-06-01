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
else:
    program = 'glslangValidator'
    compiler = 'glslc'


if platform.system() == 'Windows':
    vk_shader_program = ".\\vk_shader_analyze.exe"
    if not os.path.exists(vk_shader_program):
        debug_shader_program = ".\\Debug\\vk_shader_analyze.exe"
        if os.path.exists(debug_shader_program):
            vk_shader_program = debug_shader_program
        else:
            vk_shader_program = ".\\Release\\vk_shader_analyze.exe"
else:
    vk_shader_program = "./vk_shader_analyze"




def analyze_vk_shader(shader_path):
    """Analyze Vulkan Shader"""
    meta_content = {}
    status = subprocess.run([compiler, shader_path, "-o", shader_path+".spv", '--target-env=vulkan1.2'], capture_output=True, text=True)
    meta_content["stdout"] = status.stdout
    meta_content["stderr"] = status.stderr
    meta_content["returncode"] = status.returncode
    if status.returncode != 0:
        return json.dumps(meta_content)
    
    #execute the vulkan shader analyzer
    analyzer_status = subprocess.run([vk_shader_program, shader_path+".spv"], capture_output=True, text=True)
    meta_content["stderr"] = analyzer_status.stderr
    meta_content["stdout"] = analyzer_status.stdout
    meta_content["returncode"] = analyzer_status.returncode
    if analyzer_status.returncode != 0:
        return json.dumps(meta_content)

    print(analyzer_status.stdout)
    output = json.loads(analyzer_status.stdout)
    meta_content["in_attributes"] = output["inputs"]
    meta_content["out_attributes"] = output["outputs"] 
    meta_content["structs"] = output["structs"]
    meta_content["uniforms"] = output["uniforms"]

    json_output = json.dumps(meta_content)
    print(json_output)
    return json_output


def analyze_gl_shader(shader_path: str):
    """Analyze OpenGL shader"""
    meta_content = {}
    status = subprocess.run([program, shader_path], capture_output=True, text=True)
    meta_content["stdout"] = status.stdout
    meta_content["stderr"] = status.stderr
    meta_content["returncode"] = status.returncode
    if status.returncode != 0:
        return json.dumps(meta_content)

    uniforms = []
    in_attributes = []
    out_attributes = []
    structs = []
    compute = False
    vertex = False
    fragment = False
    extension = pathlib.Path(shader_path).suffix
    if extension == '.comp':
        compute = True
    if extension == '.vert':
        vertex = True
    if extension == '.frag':
        fragment = True
    raytracing = False
    raytracing_extensions = [".rgen", ".rmiss", "rchit", "rint", ".rahit", ".rcall"]
    if extension in raytracing_extensions:
        raytracing = True
    if raytracing:
        meta_content["returncode"] = 1
        meta_content["stderr"] = "Raytracing is not supported in OpenGL"
        return json.dumps(meta_content)

    status = subprocess.run([program, "-q", "-l", "--quiet", shader_path], capture_output=True, text=True)
    meta_content["stdout"] = status.stdout
    meta_content["stderr"] = status.stderr
    meta_content["returncode"] = status.returncode
    if status.returncode != 0:
        return json.dumps(meta_content)

    result = status.stdout
    result_lines = result.split("\n")

    class Mode(Enum):
        NONE = 0
        UNIFORM = 1
        UNIFORM_BLOCK = 2
        BUFFER_VARIABLE = 3
        BUFFER_BLOCK = 4
        PIPELINE_INPUT = 5
        PIPELINE_OUTPUT = 6
    def translate_type_name(type_name: str):
        if type_name == '8b5c':
            return 'mat4'
        if type_name == '8b51':
            return 'vec3'
        if type_name == '8b50':
            return 'vec2'
        if type_name == '8b5e':
            return 'sampler2D'
        if type_name == '8b52':
            return 'vec4'
        if type_name == '904d':
            return 'image2D'
        if type_name == '8b60':
            return 'samplerCube'
        if type_name == '1406':
            return 'float'
        if type_name == '1404':
            return 'int'
        return 'undefined'

    def get_type_name(line: str):
        split_line = line[line.index(':')+1:].split(',')
        for elem in split_line:
            if 'type' in elem:
                return translate_type_name(elem.split(' ')[-1])
        return 'undefined'

    def get_count(line: str):
        split_line = line[line.index(':')+1:].split(',')
        for elem in split_line:
            if 'size' in elem:
                return int(elem.split(' ')[-1])
        return 0

    mode = Mode.NONE
    for line in result_lines:

        if "Uniform reflection" in line:
            mode = Mode.UNIFORM
            continue
        elif "Uniform block reflection" in line:
            mode = Mode.UNIFORM_BLOCK
            continue
        elif "Buffer variable reflection" in line:
            mode = Mode.BUFFER_VARIABLE
            continue
        elif "Buffer block reflection" in line:
            mode = Mode.BUFFER_BLOCK
            continue
        elif "Pipeline input reflection" in line:
            mode = Mode.PIPELINE_INPUT
            continue
        elif "Pipeline output reflection" in line:
            mode = Mode.PIPELINE_OUTPUT
            continue

        if mode == Mode.UNIFORM:
            if ":" not in line:
                continue
            uniform_name = line[0:line.index(":")]
            uniform_obj = {"type_name": get_type_name(line), "name": uniform_name, "count": get_count(line)}
            uniforms.append(uniform_obj)
        elif mode == Mode.PIPELINE_INPUT:
            if ":" not in line:
                continue
            input_name = line[0:line.index(":")]
            in_variable = {"type_name": get_type_name(line), "name": input_name}
            in_attributes.append(in_variable)
        elif mode == Mode.PIPELINE_OUTPUT:
            if ":" not in line:
                continue
            output_name = line[0:line.index(":")]
            out_variable = {"type_name": get_type_name(line), "name": output_name}
            out_attributes.append(out_variable)
        elif mode == Mode.UNIFORM_BLOCK:
            pass
        elif mode == Mode.BUFFER_BLOCK:
            pass


    shader_content = ""
    with open(shader_path, 'r') as shader_file:
        shader_lines = shader_file.readlines()
        #remove first line
        shader_lines.pop(0)
        for line in shader_lines:
            if '//' in line:
                line = line[0:line.index('//')]
            if "void main" in line:
                break
            shader_content += line + '\n'

    shader_content = shader_content.replace('\n', '')
    shader_content = shader_content.replace('\t', '')
    shader_content = shader_content.replace('\r', '')

    lines = shader_content.split(';')
    for line in lines:
        # early exit condition
        if "void main" in line:
            break
        split_line = line.split(" ")
        if "in" in split_line and not compute:
            if vertex:
                in_index = split_line.index("in")
                input_name = split_line[in_index + 2]

                par_in_index = line.index('(')
                par_out_index = line.index(')')
                param = line[par_in_index+1:par_out_index]
                param = param.replace(' ', '')
                split_param = param.split(',')
                location = -1
                for p in split_param:
                    v = p.split('=')
                    if v[0] == 'location':
                        location = int(v[1])
                for in_attrib in in_attributes:
                    if in_attrib["name"] == input_name:
                        in_attrib["location"] = location
            else:
                in_index = split_line.index("in")
                in_variable = {"type_name": split_line[in_index + 1], "name": split_line[in_index + 2]}
                in_attributes.append(in_variable)
        if "out" in split_line and not fragment:
            out_index = split_line.index('out')
            out_variable = {"type_name": split_line[out_index + 1], "name": split_line[out_index + 2]}
            out_attributes.append(out_variable)
    meta_content["uniforms"] = uniforms
    meta_content["in_attributes"] = in_attributes
    meta_content["out_attributes"] = out_attributes
    meta_content["structs"] = structs
    #print(result)
    #print(json.dumps(meta_content, indent=2))
    return json.dumps(meta_content)

def main():
    print(analyze_gl_shader("../../gl_samples/data/shaders/scene10/model_instancing.vert"))
    print(analyze_gl_shader("../../gl_samples/data/shaders/scene11/model_instancing.vert"))
    #print(analyze_gl_shader("../../gl_samples/data/shaders/scene06/rotated_cube_ubo.vert"))
    #print(analyze_gl_shader("../../gl_samples/data/shaders/scene06/rotated_cube_ubo_block.vert"))
    #print(analyze_gl_shader("../../gl_samples/data/shaders/scene09/skybox.frag"))
    #print(analyze_gl_shader("../shaders/pre_compute_brdf.comp"))


if __name__ == '__main__':
    main()
