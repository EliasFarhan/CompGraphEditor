#pragma once
#include "engine/filesystem.h"

#include <string>

namespace core
{

    class PhysFilesystem final : public core::FilesystemInterface
    {
    public:

        PhysFilesystem(const char* argv0);
        void Begin();
        void End();
        void AddMount(std::string_view dir, std::string_view mountPoint, int append) const;
        [[nodiscard]] core::FileBuffer LoadFile(Path path) const override;
        [[nodiscard]] bool FileExists(Path path) const override;
        [[nodiscard]] bool IsRegularFile(Path path) const override;
        [[nodiscard]] bool IsDirectory(Path path) const override;
        void WriteString(Path path, std::string_view content) const override;
    private:
        std::string argv0_;

    };
} // namespace gpr5300
