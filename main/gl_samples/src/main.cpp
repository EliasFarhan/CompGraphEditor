#include "gl/engine.h"
#include "sample_program.h"
#include "py_interface.h"
#include "engine/filesystem.h"

#include <SDL_main.h>
#include <argh.h>
#include <fmt/format.h>


int main([[maybe_unused]]int argc, char** argv)
{
    argh::parser cmdl(argv);
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    gl::Engine engine;
    int major = 0, minor = 0;
    if ((cmdl({ "-M", "--major" }) >> major) && (cmdl({ "-m", "--minor" }) >> minor))
    {
        engine.SetVersion(major, minor, cmdl[{ "-es", "--es" }]);
    }
    else
    {
        LogWarning(fmt::format("Major {}, minor: {}", cmdl({ "-M", "--major" }).str(), cmdl({ "-m", "--minor" }).str()));
    }
    engine.SetWindowName("OpenGL Samples");
    core::PyManager pyManager;
    core::ImportNativeScript();

    gpr5300::SampleBrowserProgram triangleProgram;
    engine.RegisterSystem(&triangleProgram);
    engine.RegisterOnGuiInterface(&triangleProgram);
    engine.RegisterEventObserver(&triangleProgram);

    engine.Run();

    return EXIT_SUCCESS;
}
