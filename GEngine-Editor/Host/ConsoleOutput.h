#pragma once
#include <QPlainTextEdit>
#include <QStringDecoder>
#include <QTextCharFormat>

namespace GEngineEditor
{
    class ConsoleOutput final : public QPlainTextEdit
    {
    public:
        explicit ConsoleOutput(QWidget* parent = nullptr);
        void AppendBytes(const QByteArray& bytes);
        void AppendMessage(const QString& text);
        void ResetStream();
    private:
        void ApplyColor();
        void FinishLine();
        QStringDecoder m_Decoder{QStringDecoder::Utf8};
        QTextCharFormat m_Format;
        QString m_Sequence;
        int m_EscapeState = 0;
        int m_LineCharacters = 0;
        bool m_SequenceOverflow = false;
    };
}
