"""Check mandatory SDK configuration, tamper rejection and real startup failures."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk-root', required=True, type=Path)
    parser.add_argument('--logs', required=True, type=Path)
    parser.add_argument('--archive', type=Path, help='Optional official offline NuGet archive')
    parser.add_argument('--executable', type=Path, help='Rebuilt Release FrameGraphTriangle for negative startup checks')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    logs = args.logs.resolve()
    logs.mkdir(parents=True, exist_ok=True)
    sdk = args.sdk_root.resolve()
    child_env = {key: value for key, value in os.environ.items() if key.casefold() != 'path'}
    child_env['Path'] = os.environ['PATH']
    def sdk_command(directory, *definitions):
        return ['cmake', f'-DGE_AGILITY_SDK_ROOT={directory}', *definitions,
                '-P', str(root / 'BuildScripts/PrepareAgilitySDK.cmake')]

    def check(name, command, success, marker, env=None, cwd=None):
        result = subprocess.run(command, cwd=cwd or root, env=env or child_env, capture_output=True, timeout=90)
        text = (result.stdout + result.stderr).decode('utf-8', errors='replace')
        (logs / f'{name}.log').write_text(text, encoding='utf-8')
        if (result.returncode == 0) != success or marker not in text:
            raise RuntimeError(f'{name}: unexpected exit={result.returncode}; inspect {logs}')
        print(f'PASS {name}: exit={result.returncode}', flush=True)

    check('missing-config', ['cmake', '-S', str(root / 'BuildScripts/CMake'), '-B', str(logs / 'missing-config'), '-G', 'Visual Studio 17 2022', '-A', 'x64', '-DGE_WITH_MONO=OFF', '-DGE_AGILITY_SDK_ROOT='], False, 'GE_AGILITY_SDK_ROOT is required')
    env = dict(os.environ)
    env.pop('GE_AGILITY_SDK_ROOT', None)
    check('missing-premake', [str(root / 'BuildScripts/Premake/premake5.exe'), '--without-mono', 'vs2022'], False, 'GE_AGILITY_SDK_ROOT is required', env=env)
    deployment = logs / 'deployed'
    check('valid-deployment', sdk_command(sdk, '-DGE_AGILITY_CHECK_ONLY=ON', f'-DGE_AGILITY_DEPLOY_DIR={deployment}'), True, 'verified:')
    tampered = logs / 'tampered-sdk'
    shutil.copytree(sdk, tampered, dirs_exist_ok=True)
    build = logs / 'build-check'
    check('valid-build-config', ['cmake', '-S', str(root / 'BuildScripts/CMake'), '-B', str(build), '-G', 'Visual Studio 17 2022', '-A', 'x64', '-DGE_WITH_MONO=OFF', f'-DGE_AGILITY_SDK_ROOT={tampered}'], True, 'Generating done')
    target = tampered / 'D3D12Core.dll'
    with target.open('ab') as output:
        output.write(b'tampered')
    destination = logs / 'must-not-deploy'
    if destination.exists():
        raise RuntimeError('Use a fresh --logs directory for rejection checks')
    check('tampered-sdk', sdk_command(tampered, '-DGE_AGILITY_CHECK_ONLY=ON', f'-DGE_AGILITY_DEPLOY_DIR={destination}'), False, 'hash mismatch')
    if destination.exists():
        raise RuntimeError('Invalid SDK modified the deployment output')
    check('tampered-build', ['cmake', '--build', str(build), '--config', 'Debug', '--target', 'GEngine-AgilitySDKCheck'], False, 'hash mismatch')
    if args.archive:
        check('offline-prepare', sdk_command(logs / 'prepared', f'-DGE_AGILITY_ARCHIVE={args.archive.resolve()}'), True, 'verified:')
        archive = logs / 'invalid.nupkg'
        archive.write_bytes(b'invalid package')
        destination = logs / 'must-not-extract'
        check('invalid-archive', sdk_command(destination, f'-DGE_AGILITY_ARCHIVE={archive}'), False, 'archive hash mismatch')
        if destination.exists():
            raise RuntimeError('Invalid archive was extracted')
    if args.executable:
        executable = args.executable.resolve()
        binary = logs / 'negative-bin'
        binary.mkdir()
        for file in executable.parent.iterdir():
            if file.is_file() and file.suffix.lower() in ('.dll', '.exe'):
                shutil.copy2(file, binary / file.name)
        working = logs / 'negative-work'
        shutil.copytree(root / 'Example/FrameGraphTriangle/Assets', working / 'Assets')
        config = (root / 'Example/FrameGraphTriangle/Config.ini').read_text(encoding='utf-8-sig')
        import re
        config = re.sub(r'GraphicsAPI: [123]', 'GraphicsAPI: 3', config)
        (working / 'Config.ini').write_text(config, encoding='utf-8')
        env = dict(os.environ, GENGINE_D3D12_DEBUG_LAYER='1', GENGINE_EXAMPLE_FRAME_LIMIT='120')
        command = [str(binary / executable.name)]
        check('missing-runtime', command, False, 'Missing required runtime file:', env=env, cwd=working)
        shutil.copytree(sdk, binary / 'D3D12')
        shutil.copy2(Path(os.environ['SystemRoot']) / 'System32/d3d12SDKLayers.dll', binary / 'D3D12/d3d12SDKLayers.dll')
        check('mismatched-layer', command, False, 'Mismatched required runtime file:', env=env, cwd=working)


if __name__ == '__main__':
    main()
