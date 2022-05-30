import json
from zipfile import ZipFile


def export_scene(path, scene_json_txt):
	scene_json = json.loads(scene_json_txt)

	with ZipFile(path, 'w') as scene_zip:
		scene_zip.write(scene_json["scene"])

		for shader_file in scene_json["shaders"]:
			scene_zip.write(shader_file)
		for texture_file in scene_json["textures"]:
			scene_zip.write(texture_file)
		for cubemap_texture_file in scene_json["cubemap_textures"]:
			scene_zip.write(cubemap_texture_file)
		for script_file in scene_json["scripts"]:
			scene_zip.write(script_file)
		for model_file in scene_json["models"]:
			scene_zip.write(model_file)
		for other_file in scene_json["others"]:
			scene_zip.write(other_file)