#pragma once
#include <filesystem>
#include <cstdint>
#include <cwctype>

namespace GEngineEditor
{
    class ProjectContext
    {
    public:
        bool Open(const std::filesystem::path& path)
        {
            if (path.empty()) return false;
            std::error_code error;
            auto root = std::filesystem::canonical(path, error);
            if (error || !std::filesystem::is_directory(root, error) || error) return false;
            m_Root = std::move(root);
            Advance();
            return true;
        }
        void Close() { m_Root.clear(); Advance(); }
        const std::filesystem::path& Root() const { return m_Root; }
        std::uint64_t Generation() const { return m_Generation; }
        bool Contains(const std::filesystem::path& path) const
        {
            if (m_Root.empty()) return false;
            std::error_code error;
            const auto canonical = std::filesystem::canonical(path, error);
            if (error) return false;
            auto candidate = canonical.begin();
            for (const auto& component : m_Root)
            {
                if (candidate == canonical.end() || !SameComponent(component, *candidate)) return false;
                ++candidate;
            }
            return true;
        }
    private:
        static bool SameComponent(const std::filesystem::path& a, const std::filesystem::path& b)
        {
#ifdef _WIN32
            const auto& left = a.native(); const auto& right = b.native();
            if (left.size() != right.size()) return false;
            for (std::size_t i = 0; i < left.size(); ++i)
                if (std::towlower(left[i]) != std::towlower(right[i])) return false;
            return true;
#else
            return a == b;
#endif
        }
        void Advance() { if (++m_Generation == 0) ++m_Generation; }
        std::filesystem::path m_Root;
        std::uint64_t m_Generation = 0;
    };
}
