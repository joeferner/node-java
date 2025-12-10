FROM node:22.11.0-windowsservercore-ltsc2022

# Install Chocolatey and dependencies
RUN powershell -Command \
   Set-ExecutionPolicy Bypass -Scope Process -Force; \
   [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; \
   iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
RUN choco install openjdk -y --version=17.0.12
RUN choco install python3 -y
RUN choco install visualstudio2022buildtools -y
RUN choco install visualstudio2022-workload-vctools -y

# Set environment variables
ENV JAVA_HOME="C:\Program Files\OpenJDK\openjdk-17.0.12"
ENV PATH="${JAVA_HOME}\bin;${PATH}"
ENV GYP_MSVS_VERSION=2022

# Create app directory
WORKDIR C:\\app

# Copy project files
COPY . C:\\app

# Install dependencies and build
RUN npm install
RUN npm run build

# Package prebuilt binary
RUN npx node-pre-gyp package

# Default command
CMD ["cmd"]