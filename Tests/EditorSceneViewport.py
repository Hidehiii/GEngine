"""Exercise real scene pixels, frame credits, chunking, resize and scene loading."""
import argparse
import os
from pathlib import Path
import queue
import re
import struct
import subprocess
import sys
import threading
import time

sys.dont_write_bytecode = True
from D3D12QueueRegression import sample


def require(value, message="regression check failed"):
    if not value:
        raise AssertionError(message)


class Worker:
    def __init__(self, binary, log, scene=None):
        self.log = log.open("wb")
        self.process = subprocess.Popen([str(binary)] + (["--scene", str(scene)] if scene else []),
                                        cwd=binary.parent, stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE, stderr=self.log,
                                        env=dict(os.environ, GENGINE_D3D12_DEBUG_LAYER="1",
                                                 GENGINE_D3D12_INFO_QUEUE_LOGGER="1"))
        self.packets = queue.Queue(maxsize=128)
        self.request = 0
        self.reader = threading.Thread(target=self.read, daemon=True)
        self.reader.start()

    def read(self):
        def exact(size):
            data = bytearray()
            while len(data) < size:
                part = self.process.stdout.read(size - len(data))
                if not part:
                    raise EOFError("Worker pipe closed")
                data.extend(part)
            return bytes(data)
        try:
            while True:
                magic, version, kind, session, request, size, reserved = struct.unpack("<4sHHQQII", exact(32))
                if magic != b"GEIP" or version != 1 or session != 1 or size > 65536 or reserved:
                    raise RuntimeError("Invalid packet header")
                self.packets.put((kind, request, exact(size)))
        except Exception as error:
            self.packets.put(error)

    def send(self, kind, payload=b""):
        self.request += 1
        self.process.stdin.write(struct.pack("<4sHHQQII", b"GEIP", 1, kind, 1, self.request, len(payload), 0) + payload)
        self.process.stdin.flush()
        return self.request

    def receive(self):
        packet = self.packets.get(timeout=30)
        if isinstance(packet, Exception):
            raise packet
        return packet

    def ready(self):
        request = self.send(1, b"GEngine.EditorDiagnostic/1")
        require(self.receive() == (2, request, b"GEngine.EditorEngine/2"), "handshake")

    def ping(self):
        request = self.send(3)
        require(self.receive() == (4, request, b""), "independent heartbeat")

    def frame(self, width, height):
        request = self.send(12, struct.pack("<II", width, height))
        require(self.receive() == (13, request, struct.pack("<III", width, height, width * 4)), "frame begin")
        pixels = bytearray()
        while True:
            kind, reply, payload = self.receive()
            require(reply == request, "frame generation")
            if kind == 15:
                require(not payload and len(pixels) == width * height * 4, "complete image")
                return pixels
            require(kind == 14 and payload and len(pixels) + len(payload) <= width * height * 4, "bounded chunk")
            pixels.extend(payload)

    def close(self, graceful=True):
        try:
            if graceful and self.process.poll() is None:
                request = self.send(5)
                require(self.receive() == (6, request, b""), "shutdown ack")
                require(self.process.wait(timeout=10) == 0, "normal shutdown")
        finally:
            if self.process.poll() is None:
                self.process.kill()
            self.process.wait()
            self.process.stdout.close()
            self.process.stdin.close()
            self.reader.join(timeout=2)
            self.log.close()


def pixel(image, width, x, y):
    return image[(y * width + x) * 4:(y * width + x + 1) * 4]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("worker", type=Path)
    parser.add_argument("--backend", type=int, choices=(1, 2, 3), required=True)
    parser.add_argument("--logs", type=Path, required=True)
    args = parser.parse_args()
    binary = args.worker.resolve()
    logs = args.logs.resolve()
    logs.mkdir(parents=True, exist_ok=True)
    config = binary.parent / "Config.ini"
    original = config.read_bytes()
    settings, count = re.subn(rb"GraphicsAPI: [123]", f"GraphicsAPI: {args.backend}".encode(), original)
    require(count == 1, "regression check failed")
    try:
        config.write_bytes(settings)
        worker = Worker(binary, logs / "live.log")
        ok = False
        try:
            worker.ready()
            worker.ping()
            try:
                unexpected = worker.packets.get(timeout=0.3)
                raise AssertionError(f"Unsolicited frame without credit: {unexpected}")
            except queue.Empty:
                pass
            first = worker.frame(640, 360)
            require(pixel(first, 640, 160, 180)[0] > 220, "shared image component red quad")
            require(pixel(first, 640, 2, 2)[2] < 100, "background")
            started = time.monotonic()
            changed = False
            for _ in range(30):
                image = worker.frame(640, 360)
                changed |= image != first
            fps = 30 / (time.monotonic() - started)
            require(changed, "scene transform must change rendered pixels without ping")
            counters = []
            for cycle in range(40):
                width, height = ((320, 240), (1280, 720), (801, 451), (640, 360))[cycle % 4]
                image = worker.frame(width, height)
                require(pixel(image, width, width // 4, height // 2)[0] > 220, "pixels survived target replacement")
                counters.append((cycle, *(sample(worker.process) or (0, 0))))
            worker.ping()
            (logs / "memory.csv").write_text("resize,handles,private_bytes\n" +
                "".join(f"{i},{h},{m}\n" for i, h, m in counters), encoding="utf-8")
            print(f"PASS backend {args.backend}: 71 changing/sized frames, {fps:.1f} FPS unpaced; "
                  f"resize handles {counters[4][1]} -> {counters[-1][1]}, "
                  f"private bytes {counters[4][2]} -> {counters[-1][2]}")
            ok = True
        finally:
            worker.close(ok)
        # A Unicode path proves the Host/child boundary retains the actual selected file.
        scene = logs / "场景.GEScene"
        scene.write_bytes((binary.parent / "Assets/Scenes/ColorScene.GEScene").read_bytes())
        worker = Worker(binary, logs / "scene.log", scene)
        ok = False
        try:
            worker.ready()
            image = worker.frame(640, 360)
            require(pixel(image, 640, 480, 135)[2] > 230, "loaded cyan quad is above center")
            require(pixel(image, 640, 480, 245)[2] < 100, "loaded scene orientation")
            require(worker.frame(640, 360) == image, "loaded edit scene is not sample animation")
            ok = True
        finally:
            worker.close(ok)
        unsupported = logs / "unsupported.GEScene"
        unsupported.write_text("Scene: Unsupported\nGameObjects:\n  - GameObject: 1\n    Camera: {}\n", encoding="utf-8")
        worker = Worker(binary, logs / "unsupported.log", unsupported)
        try:
            worker.send(1, b"GEngine.EditorDiagnostic/1")
            require(worker.process.wait(timeout=30) != 0, "unsupported scene must fail explicitly")
        finally:
            worker.close(False)
        for name in ("live", "scene"):
            text = (logs / f"{name}.log").read_text(encoding="utf-8", errors="replace")
            require("D3D12 Error:" not in text and "D3D12 Warning:" not in text and "Validation Error" not in text, name)
        require("Unsupported scene component: Camera" in (logs / "unsupported.log").read_text(encoding="utf-8", errors="replace"), "regression check failed")
        print("PASS: scene file, Unicode path, orientation, explicit unsupported component and shutdown")
    finally:
        config.write_bytes(original)


if __name__ == "__main__":
    main()
