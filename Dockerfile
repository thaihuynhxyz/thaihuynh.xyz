FROM gcc:9

expose 443

COPY . /usr/src/thaihuynh.xyz
WORKDIR /usr/src/thaihuynh.xyz

RUN apt-get update
RUN apt-get install -y cmake
RUN apt-get install -y libmicrohttpd-dev
RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN make
RUN echo "thaihuynh_xyz:2345:respawn:/usr/src/thaihuynh.xyz/thaihuynh_xyz" > /etc/inittab

CMD ["./thaihuynh_xyz"]
