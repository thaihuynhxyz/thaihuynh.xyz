FROM gcc:9

expose 443

COPY . /usr/src/thaihuynh.xyz
WORKDIR /usr/src/thaihuynh.xyz

RUN apt-get update \
    && apt-get install -y cmake \
    && apt-get install -y libmicrohttpd-dev \
    && cmake -DCMAKE_BUILD_TYPE=Release . \
    && make \
    && echo "thaihuynh_xyz:2345:respawn:/usr/src/thaihuynh.xyz/thaihuynh_xyz" > /etc/inittab

CMD ["./thaihuynh_xyz"]
