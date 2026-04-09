## WingFlasher Build Notes

PyInstaller builds are not cross-platform. Build the Windows binary on Windows, the Linux binary on Linux, and the macOS binary on macOS.

- Windows: `dist/WingFlasher.exe`
- Linux: `dist/WingFlasher`
- macOS: `dist/WingFlasher`

To build the app with PyInstaller using `uv`, first sync dependencies:

**UPDATE ".python-version" WITH YOUR PYTHON VERSION**

``` bash
uv sync --group dev
```

Then build the project:

```bash
uv run pyinstaller WingFlasher.spec --noconfirm
```
This spec is configured for a `onefile` build. PyInstaller will produce a single `WingFlasher.exe`, while still bundling the `assets/`, `firmware/`, and default `config/` files inside the executable.

This sets the `.exe` icon and also bundles `assets/` so the running Qt app can use the same icon for its window/taskbar icon. If you want to build from the checked-in PyInstaller spec instead, use:

```bash
uv run pyinstaller WingFlasher.spec
```

The bundled firmware files from `firmware/` let flashing work in both development and PyInstaller builds.
On first launch, the app creates a `config/` directory next to the built `.exe` if needed and copies bundled default `commands.txt` and `params.txt` files there. After that, the external files beside the `.exe` are used as the editable config source.

To simply run the python app using `uv`, try

``` bash
uv run ./src/WingFlasher/main.py
```
