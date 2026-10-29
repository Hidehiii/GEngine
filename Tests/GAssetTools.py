"""Exercise the native GAsset CLI; Python is used only by this regression harness."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import yaml


def check(executable):
    def run(root, *options, success=True):
        environment = dict(os.environ)
        environment['PATH'] = ''
        result = subprocess.run(
            [str(executable), 'gasset', '--root', str(root), *options],
            env=environment, capture_output=True, text=True)
        assert (result.returncode == 0) == success, (result.stdout, result.stderr)

    with tempfile.TemporaryDirectory(prefix='native gasset ') as directory:
        root = Path(directory)
        project = root / 'Example' / 'Demo'
        assets = project / 'Assets'
        assets.mkdir(parents=True)
        text = '\tvoid main() {}  \r\n// Unicode: 中文\n\n'
        (assets / 'same.glsl').write_bytes(text.encode())
        binary = bytes(range(256)) + b'\x00\xff'
        (assets / 'same.png').write_bytes(binary)
        (assets / 'empty.ttf').write_bytes(b'')
        (assets / 'empty.glsl').write_bytes(b'')
        (assets / 'scene.GEScene').write_text('Scene: Assets/same.png\n')
        refs = project / 'references.txt'
        refs.write_text('Assets/same.png Assets/same.glsl Assets/scene.GEScene\r\n')
        ignored = assets / 'Cache'
        ignored.mkdir()
        (ignored / 'skip.png').write_bytes(binary)
        run(root, '--help')
        run(root, '--python', 'python.exe', success=False)
        run(root, '--unknown', success=False)
        run(root)
        shader = yaml.safe_load((assets / 'same_shader.gasset').read_text(encoding='utf-8'))
        texture = yaml.safe_load((assets / 'same_texture.gasset').read_text())
        assert shader['payload'] == text
        assert texture['payload'] == binary
        assert texture['type'] == 'texture' and texture['format'] == 'png'
        assert yaml.safe_load((assets / 'empty_shader.gasset').read_text())['payload'] == ''
        assert yaml.safe_load((assets / 'empty_font.gasset').read_text())['payload'] == b''
        assert (assets / 'scene.gscene').read_text() == 'Scene: Assets/same_texture.gasset\n'
        assert (ignored / 'skip.png').exists()
        assert not (assets / 'same.png').exists()
        assert refs.read_bytes().endswith(b'\r\n')
        refs.write_text('Assets/same_shader.glsl Assets/scene.GEScene')
        run(root, '--repair-references')
        assert refs.read_text() == 'Assets/same_shader.gasset Assets/scene.gscene'
        run(root)
        (assets / 'empty_shader.glsl').write_text('collision')
        before = refs.read_bytes()
        run(root, success=False)
        assert (assets / 'empty_shader.glsl').read_text() == 'collision' and refs.read_bytes() == before
        (assets / 'empty_shader.glsl').unlink()
        (assets / 'invalid.glsl').write_bytes(b'\xff')
        run(root, success=False)
        assert (assets / 'invalid.glsl').exists() and not (assets / 'invalid.gasset').exists()
        imported = root / 'single.gasset'
        source = root / 'single.png'
        source.write_bytes(binary)
        def single_import(success=True):
            environment = dict(os.environ)
            environment['PATH'] = ''
            result = subprocess.run([str(executable), 'import-resource', '--source', str(source),
                                     '--output', str(imported)], env=environment, capture_output=True)
            assert (result.returncode == 0) == success, result.stderr
        single_import()
        assert source.read_bytes() == binary
        assert yaml.safe_load(imported.read_text())['payload'] == binary
        before = imported.read_bytes()
        single_import(success=False)
        assert imported.read_bytes() == before
        assert not list(root.glob('.gasset-import-*.tmp'))
    print(f'{executable}: native GAsset regressions passed with empty PATH')


if __name__ == '__main__':
    for argument in sys.argv[1:]:
        check(Path(argument).resolve())
