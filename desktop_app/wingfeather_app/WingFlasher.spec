# -*- mode: python ; coding: utf-8 -*-

import platform
from pathlib import Path

from PyInstaller.utils.hooks import collect_data_files


# PyInstaller does not define __file__ inside the spec namespace.
# This spec is intended to be run from the project root.
ROOT = Path.cwd()
esptool_datas = collect_data_files('esptool')
icon_path = None

if platform.system() == "Windows":
    windows_icon = ROOT / "assets" / "WingFlasher.ico"
    if windows_icon.exists():
        icon_path = str(windows_icon)

a = Analysis(
    [str(ROOT / 'src' / 'WingFlasher' / 'main.py')],
    pathex=[],
    binaries=[],
    datas=[
        (str(ROOT / 'firmware'), 'firmware'),
        (str(ROOT / 'assets'), 'assets'),
        (str(ROOT / 'config' / 'commands.txt'), 'config'),
        (str(ROOT / 'config' / 'params.txt'), 'config'),
    ] + esptool_datas,
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='WingFlasher',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=icon_path,
)
