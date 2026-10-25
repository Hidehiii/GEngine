#include "ConsoleOutput.h"
#include <QScrollBar>
#include <QTextCursor>
#include <QTextBlock>
#include <QFont>

namespace GEngineEditor
{
    static QColor TerminalColor(int index)
    {
        static const QRgb colors[] = {
            0x000000, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0xc0c0c0,
            0x808080, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff};
        return QColor::fromRgb(colors[index]);
    }
    ConsoleOutput::ConsoleOutput(QWidget* parent) : QPlainTextEdit(parent)
    {
        setObjectName("ConsoleOutput"); setReadOnly(true); setUndoRedoEnabled(false);
        setMaximumBlockCount(300);
        auto colors = palette();
        colors.setColor(QPalette::Base, QColor(28, 30, 34));
        colors.setColor(QPalette::Text, TerminalColor(7)); setPalette(colors);
    }
    void ConsoleOutput::ApplyColor()
    {
        const auto parts = m_Sequence.split(';');
        for (int i = 0; i < parts.size(); ++i)
        {
            bool valid = true;
            const int code = parts[i].isEmpty() ? 0 : parts[i].toInt(&valid);
            if (!valid) continue;
            if (code == 0) m_Format = QTextCharFormat();
            else if (code == 1) m_Format.setFontWeight(QFont::Bold);
            else if (code == 22) m_Format.setFontWeight(QFont::Normal);
            else if (code >= 30 && code <= 37) m_Format.setForeground(TerminalColor(code - 30));
            else if (code >= 90 && code <= 97) m_Format.setForeground(TerminalColor(code - 90 + 8));
            else if (code >= 40 && code <= 47) m_Format.setBackground(TerminalColor(code - 40));
            else if (code >= 100 && code <= 107) m_Format.setBackground(TerminalColor(code - 100 + 8));
            else if (code == 39) m_Format.clearForeground();
            else if (code == 49) m_Format.clearBackground();
            // Extended colors are not emitted by the worker; skip their operands as a unit.
            else if ((code == 38 || code == 48) && i + 1 < parts.size())
                i += parts[i + 1] == "5" ? 2 : parts[i + 1] == "2" ? 4 : 1;
        }
    }
    void ConsoleOutput::AppendBytes(const QByteArray& bytes)
    {
        const bool follow = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
        QTextCursor cursor(document()); cursor.movePosition(QTextCursor::End);
        QString run;
        auto flush = [&] { if (!run.isEmpty()) { cursor.insertText(run, m_Format); run.clear(); } };
        const QString text = m_Decoder(bytes);
        for (const QChar ch : text)
        {
            if (ch.unicode() == 0x1b)
            { flush(); m_EscapeState = 1; m_Sequence.clear(); m_SequenceOverflow = false; continue; }
            if (m_EscapeState == 1)
            { m_EscapeState = ch == '[' ? 2 : 0; continue; }
            if (m_EscapeState == 2)
            {
                if (ch.unicode() >= 0x40 && ch.unicode() <= 0x7e)
                {
                    if (ch == 'm' && !m_SequenceOverflow) ApplyColor();
                    m_EscapeState = 0; m_Sequence.clear(); continue;
                }
                if (ch != '\n')
                {
                    if (m_Sequence.size() < 64) m_Sequence += ch;
                    else m_SequenceOverflow = true;
                    continue;
                }
                m_EscapeState = 0; m_Sequence.clear();
            }
            if (ch == '\n') { run += ch; m_LineCharacters = 0; }
            else if ((ch == '\t' || ch.unicode() >= 0x20) && ch.unicode() != 0x7f && m_LineCharacters < 16 * 1024)
            { run += ch; ++m_LineCharacters; }
        }
        flush();
        if (follow) verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }
    void ConsoleOutput::FinishLine()
    {
        if (!document()->lastBlock().text().isEmpty())
        {
            QTextCursor cursor(document()); cursor.movePosition(QTextCursor::End);
            cursor.insertText("\n", QTextCharFormat());
        }
        m_LineCharacters = 0;
    }
    void ConsoleOutput::ResetStream()
    {
        FinishLine(); m_Decoder.resetState(); m_Format = QTextCharFormat();
        m_EscapeState = 0; m_Sequence.clear(); m_SequenceOverflow = false;
    }
    void ConsoleOutput::AppendMessage(const QString& text)
    {
        const bool follow = verticalScrollBar()->value() >= verticalScrollBar()->maximum();
        FinishLine();
        QTextCursor cursor(document()); cursor.movePosition(QTextCursor::End);
        cursor.insertText(text.left(16 * 1024).trimmed() + '\n', QTextCharFormat());
        if (follow) verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }
}
