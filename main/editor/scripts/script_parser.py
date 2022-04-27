import json

def analyze_script(script_path):
    
    classes = []
    with open(script_path) as f:
        line = f.readline()
        while line:
            line = f.readline()
            if "class" in line:
                par_index = line.find('(')
                class_name = line[5:par_index]
                class_name = class_name.replace(' ', '')
                classes.append(class_name)

    script_json = {"classes":classes}
    return json.dumps(script_json)