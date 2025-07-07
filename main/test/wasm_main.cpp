#include <fstream>

#include <wasm3_cpp.h>

int main(int argc, char** argv) {
  try {
    wasm3::wasm_environment env;
    wasm3::wasm_runtime runtime = env.new_runtime(1024);
    const char* file_name = "data/scripts/scene03.wasm";
    std::ifstream wasm_file(file_name, std::ios::binary | std::ios::in);
    if (!wasm_file.is_open()) {
      throw std::runtime_error("Failed to open wasm file");
    }
    wasm3::wasm_module mod = env.parse_module(wasm_file);
    runtime.load(mod);

    wasm3::wasm_function begin_fn = runtime.find_function("begin");
    begin_fn.call();


    wasm3::wasm_function update_fn = runtime.find_function("update");
    update_fn.call(0.1f);
  }
  catch(std::runtime_error &e) {
    std::cerr << "WASM3 error: " << e.what() << std::endl;
    return 1;
  }
}