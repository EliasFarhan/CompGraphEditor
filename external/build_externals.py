import subprocess
import os
import sys


path = '.'
def build_wasm3():
    source_dir = "{}/wasm3".format(path)
    debug_dir = "{}/build-dbg".format(source_dir)
    release_dir = "{}/build-rel".format(source_dir)

def build_ktx():
    print("Building KTX-Software")
    source_dir = "{}/KTX-Software".format(path)
    debug_dir = "{}/build-dbg".format(source_dir)
    release_dir = "{}/build-rel".format(source_dir)
    os.makedirs(debug_dir, exist_ok=True)
    os.system('cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF'.format(source_dir, debug_dir))
    os.system('cmake --build {} --target ktx --config Debug --parallel'.format(debug_dir))
    os.system('cmake --build {} --target ktx_read --config Debug --parallel'.format(debug_dir))
    os.makedirs(release_dir, exist_ok=True)
    os.system('cmake -S {} -B {} -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF'.format(source_dir, release_dir))
    os.system('cmake --build {} --target ktx --config Release --parallel'.format(release_dir))
    os.system('cmake --build {} --target ktx_read --config Release --parallel'.format(release_dir))

if __name__ == "__main__":
    if len(sys.argv) > 1:
        path = sys.argv[1]
    build_ktx()
    build_wasm3()