<img src="data/stw.svg" alt="stw logo" width="50" height="50">

# stw (Stopwatch) 

Minimal GTK4 stopwatch for XFCE and other Linux desktops.  
Tracks time with centisecond precision. Lives in your taskbar, stays out of your way.

<img src="data/gtk-box.png" alt="stw visual preview screenshot" width="250">

## Keybinds

| Key | Action |
|-----|--------|
| `Space` | Start / Stop |
| `Shift+C` | Reset |

---

## Install a release (no compiler needed)

Download the latest tarball from the [Releases](../../releases/latest) page.

```bash
tar -xzf stw-*-linux-x86_64.tar.gz
cd stw-*-linux-x86_64
sudo ./install.sh
```

To remove it:

```bash
sudo rm /usr/local/bin/stw
sudo rm /usr/local/share/applications/stw.desktop
sudo rm /usr/local/share/icons/hicolor/scalable/apps/stw.svg
```

---

## Build from source

### Dependencies

**Debian / Ubuntu / Mint / Pop!_OS**

```bash
sudo apt install build-essential pkg-config libgtk-4-dev
```

**Fedora / RHEL**

```bash
sudo dnf install gcc make pkg-config gtk4-devel
```

**Arch**

```bash
sudo pacman -S base-devel pkg-config gtk4
```

### Build and install

```bash
git clone https://github.com/grgorien/stw
cd stw 
make release
sudo make install
```

Verify it worked:

```bash
stw --version
```

### Uninstall

```bash
sudo make uninstall
```

---

## Development

```bash
make          # debug build with -g -O0 (runs as ./stw)
make debug    # same as above, explicit
make release  # optimised build with -O2 (runs as ./stw)
make clean    # remove build artefacts
```

The default `make` produces a debug binary in the project directory, nothing is installed to your system. Use `sudo make install` when you want the binary available system-wide.

GTK4 is required. This will not build or run on GTK3.
