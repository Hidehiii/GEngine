#pragma once
#include <cstdint>
#include <string>
#include <utility>

namespace GEngineEditor
{
    class SelectionService
    {
    public:
        bool Select(std::uint64_t scope, std::string id)
        {
            if (scope == 0 || id.empty()) return false;
            m_Scope = scope;
            m_Id = std::move(id);
            return true;
        }
        bool Validate(std::uint64_t scope, bool itemExists)
        {
            if (m_Id.empty()) return false;
            if (scope != m_Scope || !itemExists)
            {
                Clear();
                return false;
            }
            return true;
        }
        void Clear()
        {
            m_Id.clear();
            m_Scope = 0;
        }
        void ClearScope(std::uint64_t scope)
        {
            if (scope != 0 && scope == m_Scope) Clear();
        }
        const std::string& Id() const { return m_Id; }
        std::uint64_t Scope() const { return m_Scope; }
        bool Empty() const { return m_Id.empty(); }
    private:
        std::uint64_t m_Scope = 0;
        std::string m_Id;
    };
}
