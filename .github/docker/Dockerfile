# Base on Fedora 38
FROM fedora:38
# Make sure all sources are up to date
RUN dnf update -y && \
    dnf install -y \ 
    gtkmm4.0-devel libadwaita-devel libcurl-devel g++ clang cmake git openssl-devel \
    libsoup3-devel sqlite-devel ninja-build \
    gdb make libasan libubsan liblsan libtsan \
    flatpak flatpak-builder meson \
    --nodocs --setopt install_weak_deps=False && \
    dnf clean all -y