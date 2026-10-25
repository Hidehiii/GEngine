#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace GEngineEditor
{
    class DraftDocument
    {
    public:
        DraftDocument() { Reset("Untitled editor draft"); }
        void Reset(std::string text)
        {
            m_States = {{++m_NextState, std::move(text)}};
            m_Current = 0;
            m_Saved = m_States.front().Id;
        }
        void Edit(std::string text)
        {
            if (text == Text()) return;
            m_States.resize(m_Current + 1);
            m_States.push_back({++m_NextState, std::move(text)});
            m_Current = m_States.size() - 1;
            if (m_States.size() > 256) { m_States.erase(m_States.begin()); --m_Current; }
        }
        bool CanUndo() const { return m_Current != 0; }
        bool CanRedo() const { return m_Current + 1 < m_States.size(); }
        void Undo() { if (CanUndo()) --m_Current; }
        void Redo() { if (CanRedo()) ++m_Current; }
        const std::string& Text() const { return m_States[m_Current].Text; }
        bool Dirty() const { return m_States[m_Current].Id != m_Saved; }
        void MarkSaved() { m_Saved = m_States[m_Current].Id; }
    private:
        struct State { std::uint64_t Id; std::string Text; };
        std::vector<State> m_States;
        std::size_t m_Current = 0;
        std::uint64_t m_NextState = 0;
        std::uint64_t m_Saved = 0;
    };
}
