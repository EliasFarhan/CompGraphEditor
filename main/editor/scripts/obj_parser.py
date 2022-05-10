import json
import re

def get_materials_path(obj_path: str):
    materials = []
    with open(obj_path) as f:
        line = f.readline()
        while line:
            line = f.readline()
            if "mtllib" in line:
                mtl_path = line[6:]
                mtl_path = re.sub(r'[^a-zA-Z0-9./]', '', mtl_path)
                materials.append(mtl_path)

    material_json = {"materials":materials}
    return json.dumps(material_json)