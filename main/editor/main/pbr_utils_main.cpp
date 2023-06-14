#include <argh.h>
#include <SDL_main.h>

#include "engine/filesystem.h"
#include "gl/engine.h"

#include "pbr_utils.h"

class PbrUtilSystem : public core::System
{
public:
    PbrUtilSystem(std::string_view hdrFile): hdrFile_(hdrFile)
    {
        
    }
    void Begin() override;
    void Update(float dt) override;
    void End() override;
private:
    std::string hdrFile_;
};

void PbrUtilSystem::Begin()
{
    editor::GeneratePreFilterEnvMap(hdrFile_);
}

void PbrUtilSystem::Update(float dt)
{
}

void PbrUtilSystem::End()
{
}

int main(int argc, char** argv)
{
    argh::parser cmdl(argv);
    if (cmdl.size() < 2)
        return EXIT_FAILURE;
    core::DefaultFilesystem filesystem;
    core::FilesystemLocator::provide(&filesystem);
    gl::Engine engine{};
    engine.SetVersion(4, 5, false);

    PbrUtilSystem editor(cmdl[1]);

    engine.RegisterSystem(&editor);

    engine.Run();

    return EXIT_SUCCESS;
}
