#pragma once
#include "engine/filesystem.h"

namespace gpr5300
{

    class PhysFilesystem final : public core::FilesystemInterface
    {
    public:

        PhysFilesystem(const char* argv0);
        void Begin();
        void End();
        void AddMount(std::string_view dir, std::string_view mountPoint, int append) const;
        [[nodiscard]] core::BufferFile LoadFile(std::string_view path) const override;
        [[nodiscard]] bool FileExists(std::string_view path) const override;
        [[nodiscard]] bool IsRegularFile(std::string_view path) const override;
        [[nodiscard]] bool IsDirectory(std::string_view path) const override;
        void WriteString(std::string_view path, std::string_view content) const override;
    private:
        std::string argv0_;

    };
} // namespace gpr5300
