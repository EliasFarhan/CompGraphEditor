#include "novus/engine.h"
#include "sample.h"

int main()
{
    sample::SampleBrowser sampleBrowser;
    novus::Engine engine;
    engine.RegisterSystem(&sampleBrowser);
    engine.RegisterEventObserver(&sampleBrowser);
    engine.RegisterOnGuiInterface(&sampleBrowser);
    engine.Run();
    return 0;
}