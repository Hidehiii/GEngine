"""Bounded native handle attribution for an owned FrameGraphTriangle child.

NT structure definitions: https://github.com/winsiderss/phnt/blob/master/ntpsapi.h
No system tracing settings or unrelated processes are inspected/changed.
"""
import argparse
from collections import Counter
import ctypes as C
from ctypes import wintypes as W
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import time

from D3D12QueueRegression import sample


class HandleEntry(C.Structure):
    _fields_ = [("value", W.HANDLE), ("references", C.c_size_t), ("pointers", C.c_size_t),
                ("access", W.ULONG), ("type", W.ULONG), ("attributes", W.ULONG), ("reserved", W.ULONG)]


class TraceEntry(C.Structure):
    _fields_ = [("handle", W.HANDLE), ("process", W.HANDLE), ("thread", W.HANDLE),
                ("type", W.ULONG), ("stack", C.c_void_p * 16)]


class UnicodeString(C.Structure):
    _fields_ = [("length", W.USHORT), ("maximum", W.USHORT), ("buffer", C.c_void_p)]


class ModuleInfo(C.Structure):
    _fields_ = [("base", C.c_void_p), ("size", W.DWORD), ("entry", C.c_void_p)]


class NativeHandles:
    def __init__(self, process):
        self.handle = W.HANDLE(int(process._handle))
        self.types = {}
        self.nt = C.WinDLL("ntdll")
        self.kernel = C.WinDLL("kernel32", use_last_error=True)
        self.psapi = C.WinDLL("psapi", use_last_error=True)
        self.nt.NtQueryInformationProcess.argtypes = [W.HANDLE, W.ULONG, C.c_void_p, W.ULONG, C.POINTER(W.ULONG)]
        self.nt.NtQueryInformationProcess.restype = W.LONG
        self.nt.NtSetInformationProcess.argtypes = [W.HANDLE, W.ULONG, C.c_void_p, W.ULONG]
        self.nt.NtSetInformationProcess.restype = W.LONG
        self.nt.NtQueryObject.argtypes = [W.HANDLE, W.ULONG, C.c_void_p, W.ULONG, C.POINTER(W.ULONG)]
        self.nt.NtQueryObject.restype = W.LONG
        self.nt.NtResumeProcess.argtypes = [W.HANDLE]
        self.nt.NtResumeProcess.restype = W.LONG
        self.kernel.GetCurrentProcess.restype = W.HANDLE
        self.kernel.DuplicateHandle.argtypes = [W.HANDLE, W.HANDLE, W.HANDLE, C.POINTER(W.HANDLE), W.DWORD, W.BOOL, W.DWORD]
        self.kernel.CloseHandle.argtypes = [W.HANDLE]
        self.psapi.EnumProcessModulesEx.argtypes = [W.HANDLE, C.c_void_p, W.DWORD, C.POINTER(W.DWORD), W.DWORD]
        self.psapi.GetModuleInformation.argtypes = [W.HANDLE, W.HMODULE, C.POINTER(ModuleInfo), W.DWORD]
        self.psapi.GetModuleFileNameExW.argtypes = [W.HANDLE, W.HMODULE, W.LPWSTR, W.DWORD]

    @staticmethod
    def check(status, operation):
        if status < 0:
            raise RuntimeError(f"{operation} NTSTATUS={status & 0xffffffff:#x}")

    def enable_trace(self):
        settings = (W.ULONG * 2)(0, 4096)
        self.check(self.nt.NtSetInformationProcess(self.handle, 32, settings, C.sizeof(settings)), "Enable child handle tracing")

    def resume(self):
        self.check(self.nt.NtResumeProcess(self.handle), "Resume owned child")

    def snapshot(self):
        size = 65536
        while size <= 64 * 1024 * 1024:
            buffer = C.create_string_buffer(size)
            needed = W.ULONG()
            status = self.nt.NtQueryInformationProcess(self.handle, 51, buffer, size, C.byref(needed))
            if status >= 0:
                count = C.c_size_t.from_buffer(buffer).value
                offset = 2 * C.sizeof(C.c_size_t)
                if offset + count * C.sizeof(HandleEntry) > size:
                    raise RuntimeError("Invalid native handle snapshot size")
                entries = (HandleEntry * count).from_buffer(buffer, offset)
                return {entry.value: entry.type for entry in entries}
            if status & 0xffffffff not in (0xc0000004, 0xc0000023):
                self.check(status, "Query owned child handles")
            size = max(size * 2, needed.value)
        raise RuntimeError("Handle snapshot exceeded bounded buffer")

    def type_name(self, value, index):
        if index in self.types:
            return self.types[index]
        duplicate = W.HANDLE()
        if not self.kernel.DuplicateHandle(self.handle, W.HANDLE(value), self.kernel.GetCurrentProcess(), C.byref(duplicate), 0, False, 2):
            return f"Type{index}"
        try:
            buffer = C.create_string_buffer(4096)
            needed = W.ULONG()
            status = self.nt.NtQueryObject(duplicate, 2, buffer, len(buffer), C.byref(needed))
            if status < 0:
                return f"Type{index}"
            name = UnicodeString.from_buffer(buffer)
            text = C.wstring_at(name.buffer, name.length // 2)
            self.types[index] = text
            return text
        finally:
            self.kernel.CloseHandle(duplicate)

    def modules(self):
        modules = (W.HMODULE * 1024)()
        needed = W.DWORD()
        if not self.psapi.EnumProcessModulesEx(self.handle, modules, C.sizeof(modules), C.byref(needed), 3):
            raise C.WinError(C.get_last_error())
        result = []
        for module in modules[:needed.value // C.sizeof(W.HMODULE)]:
            info = ModuleInfo()
            path = C.create_unicode_buffer(32768)
            if self.psapi.GetModuleInformation(self.handle, module, C.byref(info), C.sizeof(info)):
                self.psapi.GetModuleFileNameExW(self.handle, module, path, len(path))
                result.append((info.base, info.base + info.size, path.value))
        return result

    def trace(self, value, modules):
        buffer = C.create_string_buffer(1024 * 1024)
        C.c_void_p.from_buffer(buffer).value = value
        needed = W.ULONG()
        status = self.nt.NtQueryInformationProcess(self.handle, 32, buffer, len(buffer), C.byref(needed))
        if status < 0:
            return {"status": hex(status & 0xffffffff)}
        count = W.ULONG.from_buffer(buffer, C.sizeof(C.c_void_p)).value
        traces = []
        for i in range(min(count, (len(buffer) - 16) // C.sizeof(TraceEntry))):
            entry = TraceEntry.from_buffer(buffer, 16 + i * C.sizeof(TraceEntry))
            if entry.handle != value or entry.type != 1:
                continue
            stack = []
            for address in entry.stack:
                if not address:
                    continue
                label = hex(address)
                for start, end, name in modules:
                    if start <= address < end:
                        label = f"{Path(name).name}+{address-start:#x}"
                        break
                stack.append(label)
            traces.append(stack)
        return {"open_stacks": traces}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--logs", required=True, type=Path)
    parser.add_argument("--frames", type=int, default=30000)
    parser.add_argument("--layer", choices=(0, 1), type=int, default=1)
    parser.add_argument("--trace", action="store_true", help="Enable bounded handle stacks only in the owned child")
    parser.add_argument("--max-event-growth", type=int, help="Fail if post-warmup Event growth exceeds this budget")
    parser.add_argument("--required-sdk", type=int, help="Require an app-local Agility SDK and its full validation flags")
    args = parser.parse_args()
    if C.sizeof(C.c_void_p) != 8:
        raise RuntimeError("Use 64-bit Python for the native x64 handle snapshot")
    executable = args.executable.resolve()
    if not executable.is_file() or not 120 <= args.frames <= 60000 or (args.max_event_growth is not None and args.max_event_growth < 0):
        raise RuntimeError("Choose a native executable and 120..60000 frames")
    directory = args.logs.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    assets = Path(__file__).resolve().parents[1] / "Example/FrameGraphTriangle"
    shutil.copytree(assets / "Assets", directory / "Assets", dirs_exist_ok=True)
    config = re.sub(r"GraphicsAPI: [123]", "GraphicsAPI: 3", (assets / "Config.ini").read_text(encoding="utf-8-sig"))
    (directory / "Config.ini").write_text(config, encoding="utf-8")
    env = dict(os.environ)
    for key in ("GENGINE_EXAMPLE_SECONDS", "GENGINE_EXAMPLE_MSAA_CHECK", "GENGINE_EXAMPLE_RESOURCE_CHECK",
                "GENGINE_EXAMPLE_IMGUI", "GENGINE_EXAMPLE_SYNTHETIC_RESIZE", "GENGINE_EXAMPLE_DISCARD_RECORDING"):
        env.pop(key, None)
    env.update(GENGINE_D3D12_DEBUG_LAYER=str(args.layer), GENGINE_D3D12_INFO_QUEUE_LOGGER="1",
               GENGINE_D3D12_LIFETIME_TRACE="1", GENGINE_D3D12_LIVE_OBJECTS="1",
               GENGINE_EXAMPLE_FRAME_LIMIT=str(args.frames), GENGINE_EXAMPLE_NO_REPLACEMENT="1")
    started = time.monotonic()
    samples = []
    previous = {}
    with (directory / "runtime.log").open("wb") as log:
        child = subprocess.Popen([str(executable)], cwd=directory, env=env, stdout=log, stderr=subprocess.STDOUT,
                                 creationflags=4 if args.trace else 0)
        try:
            native = NativeHandles(child)
            if args.trace:
                native.enable_trace()
                native.resume()
            next_sample = .5
            while child.poll() is None:
                elapsed = time.monotonic() - started
                if elapsed > 120:
                    raise RuntimeError("Owned diagnostic child timed out")
                if elapsed >= next_sample:
                    try:
                        current = native.snapshot()
                        modules = native.modules()
                    except (OSError, RuntimeError) as error:
                        if child.poll() is not None:
                            break
                        if isinstance(error, OSError) and error.winerror == 299:
                            # The loader can disappear just before the process exit is signaled.
                            try:
                                child.wait(timeout=1)
                            except subprocess.TimeoutExpired:
                                raise error
                            break
                        raise
                    counts = Counter(native.type_name(handle, index) for handle, index in current.items())
                    record = {"seconds": elapsed, "memory": sample(child), "types": dict(counts)}
                    record["runtime_modules"] = [{"path": name, "base": hex(start)} for start, _, name in modules if "d3d12" in Path(name).name.lower()]
                    if args.trace:
                        added = sorted(set(current) - set(previous), reverse=True)
                        record["new_traces"] = [{"type": native.type_name(handle, current[handle]), **native.trace(handle, modules)} for handle in added[:24]]
                    samples.append(record)
                    print(f"sample seconds={elapsed:.1f} events={counts.get('Event',0)} memory={record['memory']}", flush=True)
                    previous = current
                    next_sample += 2
                time.sleep(.05)
        finally:
            if child.poll() is None:
                child.kill()
            child.wait()
            (directory / "native-handles.json").write_text(json.dumps(samples, indent=2), encoding="utf-8")
    text = (directory / "runtime.log").read_text(encoding="utf-8", errors="replace")
    if child.returncode or text.count("readback passed after 119 frames.") != 3 or f"completed {args.frames} frames" not in text:
        raise RuntimeError(f"Diagnostic child failed: exit={child.returncode}; inspect {directory}")
    if any(marker in text for marker in ("D3D12 Error:", "D3D12 Warning:")):
        raise RuntimeError(f"Native validation diagnostic: {directory}")
    if args.required_sdk is not None:
        marker = f"D3D12 Agility device: SDK={args.required_sdk}, debug={'true' if args.layer else 'false'}, synchronizedQueueValidation=true"
        if marker not in text:
            raise RuntimeError(f"Required Agility SDK/validation configuration was not applied: {directory}")
        loaded = {str(Path(module['path']).resolve()).casefold()
                  for row in samples for module in row['runtime_modules']}
        for name in ('D3D12Core.dll', 'd3d12SDKLayers.dll') if args.layer else ('D3D12Core.dll',):
            if str((executable.parent / 'D3D12' / name).resolve()).casefold() not in loaded:
                raise RuntimeError(f"Required app-local {name} was not observed: {directory}")
    steady = [row for row in samples if 3 <= row["seconds"] <= time.monotonic() - started - 1]
    if args.max_event_growth is not None:
        if len(steady) < 2:
            raise RuntimeError("Insufficient post-warmup samples for handle-growth acceptance; use more frames")
        if not all("Event" in row["types"] for row in steady):
            raise RuntimeError("Native Event type attribution is missing")
        growth = max(row["types"].get("Event", 0) for row in steady) - min(row["types"].get("Event", 0) for row in steady)
        if growth > args.max_event_growth:
            raise RuntimeError(f"Event handle growth {growth} exceeds budget {args.max_event_growth}")
        if args.layer and "Starting D3D12 debug info queue logger thread." not in text:
            raise RuntimeError("Requested validation was not verified")
    print(f"PASS bounded native handle capture: frames={args.frames}, layer={args.layer}, trace={args.trace}", flush=True)


if __name__ == "__main__":
    main()
