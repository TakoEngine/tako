FROM fedora:40

RUN sudo dnf install -y wget
COPY ./install-cmake.sh /tmp/
RUN chmod +x /tmp/install-cmake.sh && /tmp/install-cmake.sh && rm /tmp/install-cmake.sh

RUN useradd -m -G wheel -s /bin/bash tako && echo '%wheel ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER tako

RUN sudo dnf install -y git python openssh-server xauth
RUN sudo dnf install -y clang clang-tools-extra ninja-build
RUN sudo dnf install -y libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel
RUN sudo dnf install -y vim
