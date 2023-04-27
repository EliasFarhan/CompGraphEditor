import platform
import subprocess
import os
import json
import re
import pathlib

if platform.system() == 'Windows':
    vulkan_path = os.getenv("VULKAN_SDK")
    program = '{}\\Bin\\glslangValidator.exe'.format(vulkan_path)
else:
    program = 'glslangValidator'


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
    status = subprocess.run([program, "-V", shader_path, "-o", shader_path+".spv"], capture_output=True, text=True)
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


def analyze_struct(struct_txt: str):
    #get struct name
    begin_index = struct_txt.find("struct")
    end_index = struct_txt.find("{")
    struct_name = struct_txt[begin_index+6:end_index]
    struct_name = re.sub(r'[^a-zA-Z0-9]', '', struct_name)

    struct_content = struct_txt[end_index+1:struct_txt.find("}")]
    struct_content = re.sub(r'[^a-zA-Z0-9 ;]', '', struct_content)
    struct_lines = list(filter(None, struct_content.split(";")))
    attributes = []
    for line in struct_lines:
        if len(line) == 0:
            continue
        line_split = list(filter(None, line.split(" ")))
        if len(line_split) < 2:
            continue
        attributes.append({"type": line_split[0], "name": line_split[1]})
    return {"name": struct_name, "attributes": attributes}


def analyze_shader(shader_path):
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
    extension = pathlib.Path(shader_path).suffix
    if extension == '.comp':
        compute = True
    raytracing = False
    raytracing_extensions = [".rgen", ".rmiss", "rchit", "rint", ".rahit", ".rcall"]
    if extension in raytracing_extensions:
        raytracing = True
    if not compute and not raytracing:
        with open(shader_path, 'r') as shader_file:
            reading_struct = False
            lines = shader_file.readlines()
            for line in lines:
                origin_line = line
                line = line.replace('\n', '')
                line = line.replace(';', '')
                split_line = line.split(' ')

                if reading_struct:
                    struct_txt += origin_line
                    if "};" in origin_line:
                        reading_struct = False
                        structs.append(analyze_struct(struct_txt))

                if "uniform" in split_line:
                    uniform_obj = {"type_name": split_line[1], "name": split_line[2]}
                    uniforms.append(uniform_obj)
                if "in" in split_line and not compute:
                    comment_index = len(split_line)
                    if '//' in split_line:
                        comment_index = split_line.index('//')
                    index = split_line.index('in')
                    if index > comment_index:
                        continue
                    in_variable = {"type_name": split_line[index + 1], "name": split_line[index + 2]}
                    in_attributes.append(in_variable)
                if "out" in split_line and not compute:
                    comment_index = len(split_line)
                    if '//' in split_line:
                        comment_index = split_line.index('//')
                    index = split_line.index('out')
                    if index > comment_index:
                        continue
                    index = split_line.index('out')
                    out_variable = {"type_name": split_line[index + 1], "name": split_line[index + 2]}
                    out_attributes.append(out_variable)
                if "struct" in origin_line:
                    reading_struct = True
                    struct_txt = origin_line

    new_uniforms = []
    for uniform in uniforms:
        for struct in structs:
            if uniform["type_name"] == struct["name"]:
                for attrib in struct["attributes"]:
                    new_uniforms.append({"type_name": attrib["type_name"], "name": "{}.{}".format(uniform["name"],
                                                                                                  attrib["name"])})
    uniforms.extend(new_uniforms)
    meta_content["uniforms"] = uniforms
    meta_content["in_attributes"] = in_attributes
    meta_content["out_attributes"] = out_attributes
    meta_content["structs"] = structs
    
    return json.dumps(meta_content)


def analyze_gl_shader(path: str):
    pass