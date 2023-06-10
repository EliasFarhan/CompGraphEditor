import json
import platform
import os
import subprocess

if platform.system() == 'Windows':
    basisu_program = ".\\basisu.exe"
    if not os.path.exists(basisu_program):
        debug_shader_program = ".\\Debug\\basisu.exe"
        if os.path.exists(debug_shader_program):
            basisu_program = debug_shader_program
        else:
            basisu_program = ".\\Release\\basisu.exe"
else:
    basisu_program = "./basisu"


def export_ktx(texture_path, other_args: str):
    args = [basisu_program]
    args.extend(other_args.split(","))
    args.append(texture_path)
    if "" in args:
        args = list(filter(lambda x: x != '', args))
    status = subprocess.run(args, capture_output=True, text=True)
    output_file = {"stdout": status.stdout, "stderr": status.stderr, "status": status.returncode, "args": str(args)}
    return json.dumps(output_file)

