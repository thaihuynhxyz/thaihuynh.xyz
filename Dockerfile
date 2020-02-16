FROM gcc
COPY . /usr/src/thaihuynh.xyz
WORKDIR /usr/src/thaihuynh.xyz
RUN apt-get update && apt-get install -y cmake libmicrohttpd-dev
RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN make
RUN echo "thaihuynh_xyz:2345:respawn:/usr/src/thaihuynh.xyz/thaihuynh_xyz" > /etc/inittab
CMD ["./thaihuynh_xyz"]
