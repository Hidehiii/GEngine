// Isolate native fence/queue handle growth without engine or presentation work.
#include <windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <wrl/client.h>
#include <psapi.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>

using Microsoft::WRL::ComPtr;
#if defined(GENGINE_PROBE_AGILITY)
extern "C"
{
    __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;
    __declspec(dllexport) const char* D3D12SDKPath = ".\\D3D12\\";
}
#endif
namespace
{
    void Check(HRESULT result)
    {
        if (FAILED(result)) throw std::runtime_error("D3D12 probe failed HRESULT=" + std::to_string(result));
    }
    struct Event
    {
        HANDLE Handle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        Event() { if (!Handle) throw std::runtime_error("CreateEvent failed"); }
        ~Event() { CloseHandle(Handle); }
    };
    void Snapshot(const char* label)
    {
        DWORD handles = 0;
        PROCESS_MEMORY_COUNTERS_EX memory{};
        memory.cb = sizeof(memory);
        if (!GetProcessHandleCount(GetCurrentProcess(), &handles) ||
            !GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory), sizeof(memory)))
            throw std::runtime_error("Process counters failed");
        std::printf("%s handles=%lu private=%zu\n", label, handles, SIZE_T(memory.PrivateUsage));
        std::fflush(stdout);
    }
}
int main(int argc, char** argv)
{
    try
    {
        if (argc != 4) throw std::runtime_error("Use: D3D12HandleProbe layer(0|1) signal|wait|event|pending-wait|pending-event|execute|reset iterations");
        const bool layer = std::string(argv[1]) == "1";
        const std::string mode = argv[2];
        const unsigned iterations = unsigned(std::strtoul(argv[3], nullptr, 10));
        if (!iterations || iterations > 1000000 || (mode != "signal" && mode != "wait" && mode != "event" &&
            mode != "pending-wait" && mode != "pending-event" && mode != "execute" && mode != "reset"))
            throw std::runtime_error("Invalid bounded probe arguments");
        if (layer)
        {
            ComPtr<ID3D12Debug> debug;
            Check(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
            debug->EnableDebugLayer();
        }
        ComPtr<ID3D12Device> device;
        Check(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
        ComPtr<ID3D12InfoQueue> info;
        if (layer) { Check(device.As(&info)); Check(info->SetMessageCountLimit(1024)); }
        ComPtr<ID3D12CommandQueue> queue;
        D3D12_COMMAND_QUEUE_DESC desc{};
        Check(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue)));
        ComPtr<ID3D12CommandQueue> producer;
        Check(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&producer)));
        ComPtr<ID3D12Fence> fence;
        Check(device->CreateFence(1, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        ComPtr<ID3D12CommandAllocator> allocator;
        Check(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)));
        ComPtr<ID3D12GraphicsCommandList> list;
        Check(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&list)));
        Check(list->Close());
        ComPtr<ID3D12Resource> resource;
        D3D12_HEAP_PROPERTIES heap{};
        heap.Type = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_DESC buffer{};
        buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        buffer.Width = 256;
        buffer.Height = buffer.DepthOrArraySize = buffer.MipLevels = 1;
        buffer.SampleDesc.Count = 1;
        buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        buffer.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        Check(device->CreateCommittedResource(&heap,D3D12_HEAP_FLAG_NONE,&buffer,D3D12_RESOURCE_STATE_COMMON,nullptr,IID_PPV_ARGS(&resource)));
        Event event;
        Snapshot("before");
        for (unsigned i = 0; i < iterations; ++i)
        {
            if (mode == "signal") Check(queue->Signal(fence.Get(), i+2));
            if (mode == "wait") Check(queue->Wait(fence.Get(), 1));
            if (mode == "event") { Check(fence->SetEventOnCompletion(1, event.Handle)); WaitForSingleObject(event.Handle, INFINITE); }
            if (mode == "pending-wait") { Check(queue->Wait(fence.Get(),i+2)); Check(producer->Signal(fence.Get(),i+2)); }
            if (mode == "pending-event") { Check(fence->SetEventOnCompletion(i+2,event.Handle)); Check(queue->Signal(fence.Get(),i+2)); WaitForSingleObject(event.Handle,INFINITE); }
            if (mode == "execute" || mode == "reset")
            {
                if (mode == "reset")
                {
                    Check(allocator->Reset());
                    Check(list->Reset(allocator.Get(),nullptr));
                    D3D12_RESOURCE_BARRIER barriers[2]{};
                    barriers[0].Type = barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                    barriers[0].Transition = {resource.Get(),0,D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_UNORDERED_ACCESS};
                    barriers[1].Transition = {resource.Get(),0,D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_COMMON};
                    list->ResourceBarrier(1,&barriers[0]);
                    list->ResourceBarrier(1,&barriers[1]);
                    Check(list->Close());
                }
                ID3D12CommandList* lists[] = {list.Get()};
                queue->ExecuteCommandLists(1,lists);
                Check(queue->Signal(fence.Get(),i+2));
                Check(fence->SetEventOnCompletion(i+2,event.Handle));
                if (WaitForSingleObject(event.Handle,INFINITE) != WAIT_OBJECT_0) throw std::runtime_error("Reuse wait failed");
            }
        }
        Snapshot("after-calls");
        Check(queue->Signal(fence.Get(), iterations+2));
        Check(fence->SetEventOnCompletion(iterations+2, event.Handle));
        if (WaitForSingleObject(event.Handle, INFINITE) != WAIT_OBJECT_0) throw std::runtime_error("Completion wait failed");
        Snapshot("after-idle");
        Sleep(200);
        Snapshot("after-idle-settle");
        if (info)
        {
            for (UINT64 i = 0; i < info->GetNumStoredMessages(); ++i)
            {
                SIZE_T size = 0;
                Check(info->GetMessage(i, nullptr, &size));
                auto bytes = std::make_unique<char[]>(size);
                auto message = reinterpret_cast<D3D12_MESSAGE*>(bytes.get());
                Check(info->GetMessage(i, message, &size));
                if (message->Severity <= D3D12_MESSAGE_SEVERITY_WARNING)
                    throw std::runtime_error(message->pDescription);
            }
        }
        std::printf("PASS mode=%s layer=%d iterations=%u\n",mode.c_str(),layer,iterations);
        list.Reset(); allocator.Reset(); resource.Reset();
        fence.Reset();
        Snapshot("after-fence-release");
        producer.Reset(); queue.Reset(); info.Reset(); device.Reset();
        Snapshot("after-device-release");
        return 0;
    }
    catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what()); return 1; }
}
