# 베이스 이미지 설정
FROM ubuntu:20.04

# 패키지 설치 및 환경 설정
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    gcc \
    libc6-dev \
    make \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 작업 디렉토리 설정
WORKDIR /app

# 클라이언트 소스 코드 복사
COPY client.c .

# 소스 코드 컴파일
RUN gcc -o client client.c -pthread

# 실행 명령어 설정
CMD ["./client"]
