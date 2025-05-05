# Builder
FROM ubuntu:22.04 AS builder
# Dependencies-Builder
ENV DEBIAN_FRONTEND=noninteractive 
SHELL ["/bin/bash", "-c"]
# Workspace
RUN apt-get update && \
    apt-get install -y \
    curl\
    wget \
    git \
    cmake \
    build-essential \
    python3-pip \
    make \
    g++ \
    libcurl4 \
    libcurl4-openssl-dev \
    aria2 \
    ffmpeg \
    libopencv-dev \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    find / -name "*.pyc" -delete 2>/dev/null || true
# Git && Make
WORKDIR /app
RUN git clone https://github.com/wjwever/duix.ai.core.git . && \
    git submodule update --init --recursive && \
    if [ ! -d build ]; then mkdir build && cd build; else cd build; fi && \
    cmake /app && \
    make -j$(nproc)

# Running Phase
FROM ubuntu:22.04
# copy from builder
COPY --from=builder /app /app
VOLUME /root/.cache/uv
ENV DEBIAN_FRONTEND=noninteractive
ENV MINIMAX_API_KEY=${MINIMAX_API_KEY} \
    LM_API_KEY=${LM_API_KEY}
ENV PATH="/root/.local/bin:${PATH}"
ENV PATH="/app/.venv/bin:/root/.local/bin:${PATH}"
SHELL ["/bin/bash", "-c"]
# Running Dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    ffmpeg \
    portaudio19-dev \
    supervisor \
    python3 \
    libportaudio2 \
    libasound2-dev \
    libopencv-dev \
    libcurl4 \
    libcurl4-openssl-dev \
    curl \
    wget \
    unzip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    find / -name "*.pyc" -delete 2>/dev/null || true
# catalog & Initialization
RUN mkdir -p /var/log/supervisor /app/roles /app/audio /app/video
# config supervisor
COPY supervisord.conf /etc/supervisor/supervisord.conf
# Download resources & models
WORKDIR /app
RUN if [ ! -d "gj_dh_res" ]; then \
    wget -q  --no-check-certificate https://cdn.guiji.ai/duix/location/gj_dh_res.zip \
    && unzip -q gj_dh_res.zip \
    && rm gj_dh_res.zip; \
    fi && \
    if [ ! -d "roles/SiYao" ]; then \
    wget -q --no-check-certificate https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/digital/model/1719194450521/siyao_20240418.zip \
    && unzip -q siyao_20240418.zip \
    && mv siyao_20240418 "roles/SiYao" \
    && rm siyao_20240418.zip; \
    fi && \
    if [ ! -d "roles/DearSister" ]; then \
    wget -q --no-check-certificate https://digital-public.obs.cn-east-3.myhuaweicloud.com/duix/digital/model/1719194007931/bendi1_0329.zip \
    && unzip -q bendi1_0329.zip \
    && mv bendi1_0329 "roles/DearSister" \
    && rm bendi1_0329.zip; \
    fi
# UV env
WORKDIR /app/audio
# Python Dependencies
COPY requirements.txt /app/audio/requirements.txt
RUN apt-get update && apt-get install aria2 -y
RUN curl -LsS https://astral.sh/uv/install.sh | sh \
    && uv version  \
    && uv init . && uv venv \
    && uv add -r /app/audio/requirements.txt \
    && mkdir -p models \
    && export HF_ENDPOINT=https://hf-mirror.com \
    && bash hfd.sh FunAudioLLM/SenseVoiceSmall \
    && mkdir -p /root/.cache && chmod 777 /root/.cache
# envs
ENV PATH="/app/audio/.venv/bin:$PATH"

# Socks
RUN rm -f /var/run/supervisor.sock

# Port
EXPOSE 8080 6001-6003
# Go!
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/supervisord.conf"]