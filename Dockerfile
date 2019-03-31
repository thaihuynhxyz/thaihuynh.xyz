FROM gcc
COPY . /usr/src/thaihuynh.xyz
WORKDIR /usr/src/thaihuynh.xyz
RUN apt-get update && apt-get install -y cmake\
    libmicrohttpd-dev\
    && cmake -DCMAKE_BUILD_TYPE=Release .\
    && make
CMD ["./thaihuynh_xyz"]
