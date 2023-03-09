FROM alpine:3.17

# Install build dependencies
RUN apk update && apk add --no-cache \
	make libc-dev lua5.3 \
	check-dev \
	gcc clang15

# Create a directory for building the library
RUN mkdir -p -m 0700 /build

# Copy necessary files and directories
COPY Makefile /build/
COPY gen/     /build/gen/
COPY test/    /build/test/

# Generate source code
WORKDIR /build
RUN make LUA=lua5.3 generate

# Set GitHub working directory and entrypoint
ARG GITHUB_WORKSPACE=/build
ENTRYPOINT ["/build/test/docker-entrypoint.sh"]
