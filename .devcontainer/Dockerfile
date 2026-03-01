FROM platformio/platformio-core:latest

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install additional tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang-format \
    git \
    curl \
    openssh-client \
    && rm -rf /var/lib/apt/lists/*

# Install Python-based tools
RUN pip3 install --no-cache-dir \
    pre-commit \
    cpplint \
    markdownlint-cli

# Create a non-root user for development (optional but recommended)
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    apt-get update && apt-get install -y sudo && \
    echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME && \
    rm -rf /var/lib/apt/lists/*

USER $USERNAME

# Install PlatformIO as the vscode user
RUN platformio update && \
    platformio platform install espressif8266

WORKDIR /workspace
