FROM node:22.11.0-windowsservercore-ltsc2022
# Install Chocolatey (Windows package manager)
RUN powershell -Command \
   Set-ExecutionPolicy Bypass -Scope Process -Force; \
   [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; \
   iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
# Install Java JDK 17 (adjust version as needed)
RUN choco install openjdk -y --version=17.0.12
# Install Windows Build Tools for node-gyp
RUN choco install python3 -y
RUN choco install visualstudio2022buildtools -y
RUN choco install visualstudio2022-workload-vctools -y
# Alternative: Use npm to install windows-build-tools (if preferred)
# RUN npm install --global windows-build-tools
# Set environment variables for Java
ENV JAVA_HOME="C:\Program Files\OpenJDK\openjdk-17.0.12"
ENV PATH="${JAVA_HOME}\bin;${PATH}"
# Set environment variables for Visual Studio Build Tools
ENV VS140COMNTOOLS="C:\BuildTools\Common7\Tools\"
ENV GYP_MSVS_VERSION=2022
# Verify installations
RUN java -version
RUN node --version
RUN npm --version
RUN python --version
# Create app directory
WORKDIR C:\\app
# Default command
CMD ["cmd"]