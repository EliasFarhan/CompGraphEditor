#include "novus/engine.h"
#include "sample.h"
#include "wasm_interface.h"

int main()
{

    core::WasmManager wasmManager;
    core::ImportNativeScript();
    sample::SampleBrowser sampleBrowser;
    novus::Engine engine;
    engine.SetShaderFormat(novus::engine::ShaderFormat_DXIL);

    engine.SetWindowName("SDL3 GPU samples");
    engine.RegisterSystem(&sampleBrowser);
    engine.RegisterEventObserver(&sampleBrowser);
    engine.RegisterOnGuiInterface(&sampleBrowser);
    engine.Run();
    return 0;
}