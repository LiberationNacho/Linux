# Base image
FROM ubuntu:20.04

# Prevent interactive prompts during package installation
ARG DEBIAN_FRONTEND=noninteractive

# 패키지 설치 및 환경 설정
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    gcc \
    libc6-dev \
    make

# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# 작업 디렉토리 설정
WORKDIR /app

# 서버 소스 코드 복사
COPY server.c .

# 소스 코드 컴파일
RUN gcc -o server server.c -lpthread

# Expose the port
EXPOSE 8080

# 실행 명령어 설정
CMD ["./server"]