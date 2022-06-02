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
    if not compute:
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
                    uniform_obj = {"type": split_line[1], "name": split_line[2]}
                    uniforms.append(uniform_obj)
                if "in" in split_line and not compute:
                    comment_index = len(split_line)
                    if '//' in split_line:
                        comment_index = split_line.index('//')
                    index = split_line.index('in')
                    if index > comment_index:
                        continue
                    in_variable = {"type": split_line[index + 1], "name": split_line[index + 2]}
                    in_attributes.append(in_variable)
                if "out" in split_line and not compute:
                    comment_index = len(split_line)
                    if '//' in split_line:
                        comment_index = split_line.index('//')
                    index = split_line.index('out')
                    if index > comment_index:
                        continue
                    index = split_line.index('out')
                    out_variable = {"type": split_line[index + 1], "name": split_line[index + 2]}
                    out_attributes.append(out_variable)
                if "struct" in origin_line:
                    reading_struct = True
                    struct_txt = origin_line

    new_uniforms = []
    for uniform in uniforms:
        for struct in structs:
            if uniform["type"] == struct["name"]:
                for attrib in struct["attributes"]:
                    new_uniforms.append({"type": attrib["type"], "name": "{}.{}".format(uniform["name"], attrib["name"])})
    uniforms.extend(new_uniforms)
    meta_content["uniforms"] = uniforms
    meta_content["in_attributes"] = in_attributes
    meta_content["out_attributes"] = out_attributes
    meta_content["structs"] = structs
    
    return json.dumps(meta_content)
