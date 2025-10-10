import external.build_externals as external_modules
import os

def setup_project():
    os.system('git submodule update --init --recursive')
    external_modules.build_all("external")

if __name__ == '__main__':
    setup_project()