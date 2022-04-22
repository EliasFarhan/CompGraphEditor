import platform
import subprocess
import os
import json

if platform.system() == 'Windows':
    vulkan_path = os.getenv("VULKAN_SDK")
    program = '{}\\Bin\\glslangValidator.exe'.format(vulkan_path)
else:
    program = 'glslangValidator'


def analyze_shader(shader_path):
    status = subprocess.run([program, shader_path])
    if status.returncode != 0:
        exit(1)

    meta_content = {}
    uniforms = []
    in_attributes = []
    out_attributes = []

    with open(shader_path, 'r') as shader_file:
        lines = shader_file.readlines()
        for line in lines:
            line = line.replace('\n', '')
            line = line.replace(';', '')
            split_line = line.split(' ')
            if "uniform" in split_line:
                uniform_obj = {"type": split_line[1], "name": split_line[2]}
                uniforms.append(uniform_obj)
            if "in" in split_line:
                comment_index = len(split_line)
                if '//' in split_line:
                    comment_index = split_line.index('//')
                index = split_line.index('in')
                if index > comment_index:
                    continue
                in_variable = {"type": split_line[index + 1], "name": split_line[index + 2]}
                in_attributes.append(in_variable)
            if "out" in split_line:
                comment_index = len(split_line)
                if '//' in split_line:
                    comment_index = split_line.index('//')
                index = split_line.index('out')
                if index > comment_index:
                    continue
                index = split_line.index('out')
                out_variable = {"type": split_line[index + 1], "name": split_line[index + 2]}
                out_attributes.append(out_variable)
    meta_content["uniforms"] = uniforms
    meta_content["in_attributes"] = in_attributes
    meta_content["out_attributes"] = out_attributes
    return json.dumps(meta_content)
