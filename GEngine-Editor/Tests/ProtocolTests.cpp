#include "Protocol/EditorProtocol.h"
#include "Core/DraftDocument.h"
#include "Core/SelectionService.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace GEngineEditor;
using namespace GEngineEditor::Protocol;
static void Require(bool value, const char* description)
{
    if (!value) throw std::runtime_error(description);
}
int main()
{
    try
    {
        Packet hello{Kind::Hello, 0x1020304050607080, 42, Text(BuildId)};
        const auto bytes = Encode(hello);
        Require(bytes[8] == 0x80 && bytes[15] == 0x10 && bytes[16] == 42, "wire byte order");
        for (std::size_t split = 0; split <= bytes.size(); ++split)
        {
            Decoder decoder;
            std::vector<Packet> packets;
            Require(decoder.Feed(bytes.data(), split, packets), "first fragment");
            Require(decoder.Feed(bytes.data() + split, bytes.size() - split, packets), "second fragment");
            Require(packets.size() == 1 && packets[0].Session == hello.Session &&
                packets[0].Request == 42 && Text(packets[0].Payload) == BuildId && !decoder.HasPartial(), "fragment decode");
        }
        auto pair = bytes; pair.insert(pair.end(), bytes.begin(), bytes.end());
        {
            const auto engine = Encode({Kind::EngineFrame, hello.Session, 0, std::vector<std::uint8_t>(57612)});
            Decoder stream; std::vector<Packet> decoded;
            for (std::size_t offset = 0; offset < engine.size(); offset += 4096)
                Require(stream.Feed(engine.data() + offset, std::min<std::size_t>(4096, engine.size() - offset), decoded),
                    "fragmented engine frame");
            Require(decoded.size() == 1 && decoded.front().Type == Kind::EngineFrame &&
                decoded.front().Payload.size() == 57612, "engine frame preserved");
        }
        Decoder decoder;
        {
            // Largest chunks cross both header and payload boundaries in the byte stream.
            const auto chunk = Encode({Kind::FrameChunk, hello.Session, 99, std::vector<std::uint8_t>(MaxPayload, 0xa5)});
            const auto end = Encode({Kind::FrameEnd, hello.Session, 99, {}});
            auto stream = chunk; stream.insert(stream.end(), end.begin(), end.end());
            Decoder chunks; std::vector<Packet> result;
            for (std::size_t offset = 0; offset < stream.size(); offset += 4093)
                Require(chunks.Feed(stream.data() + offset, std::min<std::size_t>(4093, stream.size() - offset), result),
                    "maximum chunk fragmented across stream reads");
            Require(result.size() == 2 && result[0].Request == 99 && result[0].Payload.size() == MaxPayload &&
                result[0].Payload.back() == 0xa5 && result[1].Type == Kind::FrameEnd && !chunks.HasPartial(),
                "maximum chunk and frame boundary preserved");
        }
        std::vector<Packet> packets;
        Require(decoder.Feed(pair.data(), pair.size(), packets) && packets.size() == 2, "coalesced packets");
        for (auto offset : {0, 4, 6, 28})
        {
            auto corrupt = bytes; corrupt[offset] = 0xff;
            Decoder invalid; std::vector<Packet> result;
            Require(!invalid.Feed(corrupt.data(), corrupt.size(), result), "invalid header rejected");
            Require(!invalid.Feed(bytes.data(), bytes.size(), result), "failed stream stays failed");
        }
        auto oversized = bytes; oversized[26] = 2;
        Decoder limited; std::vector<Packet> result;
        Require(!limited.Feed(oversized.data(), HeaderSize, result), "oversized payload rejected before allocation");
        Decoder truncated;
        Require(truncated.Feed(bytes.data(), bytes.size() - 1, result) && truncated.HasPartial(), "partial EOF visible");
        DraftDocument draft;
        draft.Edit("saved"); draft.MarkSaved(); draft.Edit("unsaved"); draft.Undo();
        Require(!draft.Dirty(), "undo restores save point");
        draft.Redo(); Require(draft.Dirty(), "redo leaves save point"); draft.Undo(); draft.Edit("new branch");
        Require(!draft.CanRedo() && draft.Dirty(), "edit invalidates redo branch");
        for (int i = 0; i < 300; ++i) draft.Edit(std::to_string(i));
        int undoCount = 0; while (draft.CanUndo()) { draft.Undo(); ++undoCount; }
        Require(undoCount == 255 && draft.Dirty(), "bounded history retains correct dirty state");
        SelectionService selection;
        Require(!selection.Select(0, "Assets/texture.png"), "empty scope rejected");
        Require(!selection.Select(9, {}), "empty selection id rejected");
        Require(selection.Select(9, "Assets/texture.png"), "valid selection accepted");
        Require(selection.Validate(9, true), "current project selection remains valid");
        Require(!selection.Validate(10, true) && selection.Empty(), "project switch clears selection");
        Require(selection.Select(11, "Assets/scene.txt"), "selection restored");
        selection.ClearScope(12);
        Require(!selection.Empty(), "unrelated scope does not clear selection");
        selection.ClearScope(11);
        Require(selection.Empty(), "matching scope clears selection");
        Require(selection.Select(13, "Assets/image.png"), "selection after clear accepted");
        Require(!selection.Validate(13, false) && selection.Empty(), "deleted resource clears selection");
        std::cout << "PASS: fragmented/coalesced pipes, invalid headers, limits, EOF, draft history and selection scope\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr << "FAIL: " << error.what() << '\n'; return 1; }
}
