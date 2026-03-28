FROM alpine:3.18 AS builder
RUN apk add --no-cache bash g++ musl-dev curl tar
SHELL ["/bin/bash", "-o", "pipefail", "-c"]
WORKDIR /cbt
COPY . .
RUN ./script.sh init compile build \
    && mkdir -p /opt/cbt \
    && cp build/cbt /opt/cbt/.
ENV PATH="/opt/cbt:${PATH}"

FROM alpine:3.18 AS deployment
RUN apk add --no-cache libstdc++ libgcc cppcheck
WORKDIR /app
RUN mkdir environments
COPY --from=builder /cbt/environments/.env.template environments/
COPY --from=builder /cbt/build/cbt /usr/local/bin/cbt
ENV env=production
