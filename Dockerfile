# Options: gcc, llvm
ARG TOOLCHAIN=gcc

FROM alpine:3.21 AS builder
ARG TOOLCHAIN
RUN DEPENDENCIES="bash musl-dev curl tar" \
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

FROM alpine:3.21 AS deployment
ARG TOOLCHAIN
RUN DEPENDENCIES="cppcheck" \
    && if [ "$TOOLCHAIN" = "gcc" ]; then \
            DEPENDENCIES="$DEPENDENCIES libstdc++ libgcc"; \
        else \
            DEPENDENCIES="$DEPENDENCIES libc++ libunwind"; \
        fi \
    && apk add --no-cache $DEPENDENCIES
WORKDIR /app
RUN mkdir environments
COPY --from=builder /cbt/environments/.env.template environments/
COPY --from=builder /cbt/build/cbt /usr/local/bin/cbt
ENV env=production
