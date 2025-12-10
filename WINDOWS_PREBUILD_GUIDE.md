# Building and Publishing node-pre-gyp Prebuilt Binaries in Docker

This guide shows how to use a Windows Docker container to build, package, and test prebuilt binaries for your native Node.js module using node-pre-gyp.

---

## 1. Prerequisites

- Docker Desktop on Windows, set to use Windows containers.
- Your project’s Dockerfile (see below for example).
- Write access to your GitHub repo (for uploading binaries).

---

## 2. Example Dockerfile

```dockerfile
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
```

---

## 3. Building the Docker Image

```sh
docker build -t node-java-windows .
```

---

## 4. Running the Container to Build and Package

```sh
docker run --rm -w C:\app node-java-windows cmd /c "npm install && npm run build && npx node-pre-gyp package"
```

This will:
- Install dependencies
- Build your native module
- Package the prebuilt binary (creates a `.tar.gz` file in your binary path)

---

## 5. Uploading the Binary to GitHub Releases

You can manually upload the `.tar.gz` file to your repo’s release page, or automate with GitHub CLI:

```sh
gh release create v1.0.0 ./lib/binding/nodejava-v1.0.0-node-v{node_abi}-{platform}-{arch}.tar.gz
```

Replace `v1.0.0` and the filename as appropriate.

---

## 6. Testing the Prebuilt Binary

To test that the prebuilt binary works:

1. Remove the build artifacts:
   ```sh
   rm -rf ./lib/binding/* ./build
   ```

2. Run install to fetch the prebuilt binary:
   ```sh
   npm install
   ```

3. Run your module’s tests:
   ```sh
   npm test
   ```

If everything works, your module is using the prebuilt binary.

---
### Testing Prebuilt Binary in Docker

You can also perform these steps inside your Docker container:

```sh
docker run --rm -w C:\app node-java-windows cmd /c "rm -rf ./lib/binding/* ./build && npm install && npm test"
```

This command will:
- Remove build artifacts
- Run `npm install` to fetch the prebuilt binary
- Run your tests to verify the binary works

This ensures your module works with the prebuilt binary, without rebuilding from source.

## 7. Troubleshooting

- Ensure your `package.json` is configured with the correct `binary.host` for GitHub Releases.
- The binary filename and release tag must match what node-pre-gyp expects.
- You need write access and a GitHub token for automated uploads.

---

## References

- [node-pre-gyp documentation](https://github.com/mapbox/node-pre-gyp)
- [GitHub CLI](https://cli.github.com/)
- [Docker Windows containers](https://docs.docker.com/desktop/windows/)

---

Let me know if you need further customization!