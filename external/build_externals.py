import subprocess
import os
import sys

core_number = os.cpu_count() - 1
path = '.'
def build_wasm3():
    print("Build WASM3")
    source_dir = "{}/wasm3".format(path)
    debug_dir = "{}/build-dbg".format(source_dir)
    release_dir = "{}/build-rel".format(source_dir)
    targets = ["m3", "uv_a", "uvwasi_a"]
    os.makedirs(debug_dir, exist_ok=True)
    os.system('cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF'.format(source_dir, debug_dir))
    for target in targets:
        os.system('cmake --build {} --target {} --config Debug'.format(debug_dir, target))
    os.makedirs(release_dir, exist_ok=True)
    os.system('cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF'.format(source_dir, release_dir))
    for target in targets:
        os.system('cmake --build {} --target {} --config Release'.format(release_dir, target))

def build_ktx():
    print("Building KTX-Software")
    source_dir = "{}/KTX-Software".format(path)
    debug_dir = "{}/build-dbg".format(source_dir)
    release_dir = "{}/build-rel".format(source_dir)
    os.makedirs(debug_dir, exist_ok=True)
    os.system('cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF'.format(source_dir, debug_dir))
    os.system('cmake --build {} --target ktx --config Debug --parallel {}'.format(debug_dir, core_number))
    os.system('cmake --build {} --target ktx_read --config Debug --parallel {}'.format(debug_dir, core_number))
    os.makedirs(release_dir, exist_ok=True)
    os.system('cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF'.format(source_dir, release_dir))
    os.system('cmake --build {} --target ktx --config Release --parallel {}'.format(release_dir, core_number))
    os.system('cmake --build {} --target ktx_read --config Release --parallel {}'.format(release_dir, core_number))

def build_assimp():
    print("Building Assimp")
    source_dir = "{}/assimp".format(path)
    debug_dir = "{}/build-dbg".format(source_dir)
    release_dir = "{}/build-rel".format(source_dir)
    os.makedirs(debug_dir, exist_ok=True)
    os.system('''cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_INSTALL=OFF -DASSIMP_BUILD_ASSIMP_VIEW=OFF -DASSIMP_BUILD_DRACO=ON -DASSIMP_BUILD_DRACO_STATIC=ON -DASSIMP_BUILD_ZLIB=ON'''.format(source_dir, debug_dir))
    os.system('cmake --build {} --target assimp --config Debug --parallel {}'.format(debug_dir, core_number))
    os.makedirs(release_dir, exist_ok=True)
    os.system('''cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_INSTALL=OFF -DASSIMP_BUILD_ASSIMP_VIEW=OFF -DASSIMP_BUILD_DRACO=ON -DASSIMP_BUILD_DRACO_STATIC=ON -DASSIMP_BUILD_ZLIB=ON'''.format(source_dir, release_dir))
    os.system('cmake --build {} --target assimp --config Release --parallel {}'.format(release_dir, core_number))

def build_all(from_path = "."):
    global path
    path = from_path
    build_ktx()
    build_wasm3()
    build_assimp()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        path = sys.argv[1]
    build_all(path)
