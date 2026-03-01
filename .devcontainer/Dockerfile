FROM python:3.11-slim

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install tools and GitHub CLI in a single layer
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang-format \
    curl \
    fish \
    git \
    lolcat \
    openssh-client \
    sudo \
    && curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg \
       | dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg \
    && chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" \
       | tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
    && apt-get update && apt-get install -y gh \
    && rm -rf /var/lib/apt/lists/*

# Install Python-based tools (pinned versions for reproducibility)
RUN pip3 install --no-cache-dir \
    "platformio==6.1.16" \
    "pre-commit==3.7.1" \
    "cpplint==1.6.1" \
    "nodeenv==1.8.0"

# Create a non-root user for development
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m -s /usr/bin/fish $USERNAME && \
    echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME

RUN mkdir -p /home/$USERNAME/.config/fish
COPY config.fish /home/$USERNAME/.config/fish/config.fish
RUN chown -R $USERNAME:$USERNAME /home/$USERNAME/.config

USER $USERNAME

# Pre-install the ESP8266 platform for faster first-use
RUN platformio platform install espressif8266

WORKDIR /workspace
