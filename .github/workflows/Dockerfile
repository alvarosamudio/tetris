ARG BASE_IMAGE=linuxdeepin/deepin:crimson
FROM ${BASE_IMAGE} AS builder

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        debhelper \
        fakeroot \
        qt6-base-dev \
        libdtk6widget-dev \
        qt6-multimedia-dev \
        qt6-tools-dev-tools \
        qt6-l10n-tools \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . /build

# Build the Debian package (outputs to parent directory /)
RUN dpkg-buildpackage -us -uc -b && \
    mkdir -p /output && \
    cp ../*.deb /output/

FROM scratch AS export
COPY --from=builder /output/ /