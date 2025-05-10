# Aps-UI
GTK4 Based UI for Stats all Around AP Sensing in Böblingen using a Raspberry Pi.

## Examples
![Overview in dark mode](images/overview_dark.png)

## Building

### Requirements
#### Fedora
```
sudo dnf install gtkmm4.0-devel libadwaita-devel libcurl-devel g++ clang cmake git protobuf-c-devel
sudo dnf install libsoup3-devel sqlite-devel ninja-build meson
```

### Building
```
git clone https://github.com/COM8/aps-ui.git
cd aps-ui
mkdir build
cd build
cmake ..
cmake --build .
```

## Flatpak
The aps-ui can be built and installed using Flatpak.

### Requirements
#### Fedora
```
sudo dnf install flatpak flatpak-builder
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install -y flathub runtime/org.gnome.Sdk/$(arch)/47 runtime/org.gnome.Platform/$(arch)/47
```

#### Debian/Ubuntu
```
sudo apt install flatpak flatpak-builder
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install -y flathub runtime/org.gnome.Sdk/$(arch)/47 runtime/org.gnome.Platform/$(arch)/47
```

### Building
Add `--jobs=1` to the `flatpak-builder` command when building on a Raspberry Pi to prevent running out of RAM.
```
git clone https://github.com/COM8/aps-ui.git
cd aps-ui
flatpak-builder --force-clean flatpak_build_dir de.aps_ui.cli.yml
```

### Installing
```
flatpak-builder --user --install --force-clean flatpak_build_dir de.aps_ui.cli.yml
```

### Cross-Compiling
In case you would like to cross-compile for the Raspberry Pi (aarch64), this can be done as follows:
```
sudo dnf install qemu-system-arm qemu-user-static
sudo systemctl restart systemd-binfmt.service

flatpak install flathub runtime/org.gnome.Sdk/aarch64/47 runtime/org.gnome.Platform/aarch64/47
flatpak-builder --arch=aarch64 --repo=repo --force-clean flatpak_build_dir de.aps_ui.cli.yml
flatpak build-bundle --arch=aarch64 ./repo/ de.aps_ui.cli.flatpak de.aps_ui.cli

# Copy the "repo" dir to the target system
flatpak --user remote-add --no-gpg-verify aps-ui-repo repo
flatpak --user install aps-ui-repo de.aps_ui.cli
```

### Uninstalling
```
flatpak uninstall de.aps_ui.cli
```

### Executing
```
flatpak run de.aps_ui.cli
```
