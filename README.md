# [thaihuynh.xyz](http://thaihuynh.xyz/about.html)

### Peronal website ###

* Implement personal website by [GNU Libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/)
* Version 1.2.0

### Set up ###

* Summary of set up

    * Clone the repository
    
            $ git clone https://github.com/thaihuynhxyz/thaihuynh.xyz.git
            $ cd thaihuynh.xyz
    
* Configuration

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

    docker run -p 80:8080 -t thaihuynhxyz/thaihuynh_xyz

    http://localhost:8080

* Deployment instructions

    docker build -t thaihuynh_xyz .

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin

    Owner: [thaihuynhxyz](https://github.com/thaihuynhxyz)
   
* Other community or team contact
    