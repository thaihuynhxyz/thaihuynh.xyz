# [thaihuynh.xyz](https://thaihuynh.xyz)

### Peronal website ###

* Implement personal website by [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/)
* Version 1.2.10

### Set up ###

* Summary of set up

    * Clone the repository
    
            $ git clone https://github.com/thaihuynhxyz/thaihuynh.xyz.git
            $ cd thaihuynh.xyz
    
* Configuration

    Build web pages by [jekyll](https://thaihuynh.xyz/jekyll/update/2020/02/15/welcome-to-jekyll.html)
    
        jekyll build -b "" -d ../blog

    Build by [CMake](https://cmake.org/)
    
        mkdir build
        cd build
        cmake ..
        make -j4
    
* Dependencies

    [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/)
    
    [CMake](https://cmake.org/)
    
        sudo apt-get install cmake libmicrohttpd-dev
        
* Database configuration
* How to run tests

    docker run -p 80:8080 -t thaihuynhxyz/thaihuynh_xyz:v1.2.4

    http://localhost:8080

* Deployment instructions

    docker build -t thaihuynh_xyz .
    
    docker tag thaihuynh_xyz:latest [HOSTNAME]/[PROJECT-ID]/[IMAGE]
    
    docker push [HOSTNAME]/[PROJECT-ID]/[IMAGE]

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin

    Owner: [thaihuynhxyz](https://github.com/thaihuynhxyz)
   
* Other community or team contact
    
