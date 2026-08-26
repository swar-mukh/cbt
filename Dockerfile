FROM alpine:3.21

# Options: `gcc`, `llvm`; defaults to `gcc` if `--build-arg` isn't passed
ARG TOOLCHAIN=gcc

RUN DEPENDENCIES="bash musl-dev curl tar cppcheck" \
    && if [ "$TOOLCHAIN" = "gcc" ]; then \
            DEPENDENCIES="$DEPENDENCIES g++"; \
        else \
            DEPENDENCIES="$DEPENDENCIES clang libc++-dev libc++-static llvm-libunwind-dev compiler-rt lld"; \
        fi \
    && apk add --no-cache $DEPENDENCIES

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
ENV TOOLCHAIN=${TOOLCHAIN}
WORKDIR /cbt
COPY . .

RUN ./script.sh init compile build \
    && mkdir -p /opt/cbt \
    && cp build/cbt /opt/cbt/.

ENV PATH="/opt/cbt:${PATH}"
