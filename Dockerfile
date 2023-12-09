# MIT License

# Copyright (c) 2023 Artur Wisz

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

FROM ev3dev/debian-stretch-cross 

USER compiler
ENV HOME=/home/compiler

RUN sudo sh -c "echo 'deb http://archive.debian.org/debian stretch main contrib non-free' > /etc/apt/sources.list"
RUN sudo sh -c "echo 'deb-src http://archive.debian.org/debian stretch main contrib non-free' >> /etc/apt/sources.list"
RUN sudo sh -c "echo 'deb http://archive.ev3dev.org/debian stretch main' >> /etc/apt/sources.list"
RUN sudo sh -c "echo 'deb-src http://archive.ev3dev.org/debian stretch main' >> /etc/apt/sources.list"

ARG PKG_LIST="unzip bzip2 build-essential g++ automake autoconf libtool wget git mc \
              zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev curl libbz2-dev"

RUN sudo apt update
RUN sudo apt upgrade -y
RUN sudo apt install -y ${PKG_LIST}

#
# Build python 3.7 from source, needed by fastdds tool
#
WORKDIR $HOME/python3
RUN sudo chown compiler $HOME/python3 
RUN wget https://www.python.org/ftp/python/3.7.17/Python-3.7.17.tgz
RUN tar xzf Python-3.7.17.tgz
RUN cd Python-3.7.17 && ./configure && make -j8 && sudo make install
RUN python3 --version 

# Uninstall the libssl-dev package as it conflicts with the armel package.
RUN sudo apt remove -y libssl-dev

ARG ARMEL_PKG_LIST="libssl-dev:armel"
RUN sudo apt install -y ${ARMEL_PKG_LIST}

#
# Build cmake for host
#
WORKDIR $HOME/cmake 
RUN sudo chown compiler $HOME/cmake 
RUN wget https://cmake.org/files/v3.15/cmake-3.15.7.tar.gz
RUN tar xzf cmake-3.15.7.tar.gz
RUN mkdir cmake-build && cd cmake-build && ../cmake-3.15.7/bootstrap && make -j6 && sudo make install

#
# From now on build for the EV3 target
#
ENV CC=arm-linux-gnueabi-gcc
ENV CXX=arm-linux-gnueabi-g++

#
# Build asio
#
WORKDIR $HOME/asio 
RUN sudo chown compiler $HOME/asio
RUN git clone https://github.com/chriskohlhoff/asio.git
RUN cd asio && git checkout asio-1-12-2
RUN cd asio/asio && ./autogen.sh && ./configure --host=arm-linux-gnueabi --without-boost --prefix=/opt/ev3dds && make -j6 && sudo make install 

#
# Build tinyxml2
#
WORKDIR $HOME/tinyxml
RUN sudo chown compiler $HOME/tinyxml
RUN git clone https://github.com/leethomason/tinyxml2.git
RUN mkdir tinyxml-build && cd tinyxml-build && CXXFLAGS="-fPIC" cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/ev3dds ../tinyxml2 && make -j4 && sudo make install

WORKDIR $HOME/Fast-DDS
RUN sudo chown compiler $HOME/Fast-DDS

#
# Build FastDDS
#
RUN git clone https://github.com/eProsima/foonathan_memory_vendor.git
RUN cd foonathan_memory_vendor && git checkout v1.3.1
RUN mkdir foonathan_memory_vendor/build
WORKDIR $HOME/Fast-DDS/foonathan_memory_vendor/build
RUN CXXFLAGS="-fno-strict-aliasing" cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX:PATH=/opt/ev3dds
RUN cmake --build . --target all -j6 && sudo make install

WORKDIR $HOME/Fast-DDS
RUN git clone https://github.com/eProsima/Fast-CDR.git
RUN cd Fast-CDR && git checkout v1.0.24
RUN mkdir Fast-CDR/build
WORKDIR $HOME/Fast-DDS/Fast-CDR/build
RUN CXXFLAGS="-fno-strict-aliasing" cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/opt/ev3dds
RUN cmake --build . --target all -j6 && sudo make install

WORKDIR $HOME/Fast-DDS
RUN git clone https://github.com/eProsima/Fast-DDS.git
RUN cd Fast-DDS && git checkout 2.6.x
RUN mkdir Fast-DDS/build
WORKDIR $HOME/Fast-DDS/Fast-DDS/build
RUN CXXFLAGS="-fno-strict-aliasing" cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/opt/ev3dds 
RUN cmake --build . --target all -j8 && sudo make install

WORKDIR $HOME
RUN sudo apt install -y fonts-dejavu java-common libasound2 libfreetype6 libfontconfig1 libxi6 libxrender1 libxtst6
RUN wget https://builds.openlogic.com/downloadJDK/openlogic-openjdk/11.0.21+9/openlogic-openjdk-11.0.21+9-linux-x64-deb.deb
RUN sudo dpkg -i openlogic-openjdk-11.0.21+9-linux-x64-deb.deb
RUN sudo update-alternatives --install /usr/bin/java java /usr/lib/jvm/openlogic-openjdk-11-hotspot-amd64/bin/java 2
RUN sudo update-alternatives --config java

WORKDIR $HOME/Fast-DDS
RUN git clone --recursive https://github.com/eProsima/Fast-DDS-Gen.git
RUN cd Fast-DDS-Gen && git checkout v2.5.1
RUN cd Fast-DDS-Gen && ./gradlew assemble

ENV PATH=$PATH:/usr/local/tools/fastdds:$HOME/Fast-DDS/Fast-DDS-Gen/scripts

#
# Build ev3dev C++ bindings
#
WORKDIR $HOME/ev3dev 
RUN sudo chown compiler $HOME/ev3dev
RUN git clone https://github.com/ddemidov/ev3dev-lang-cpp.git
WORKDIR $HOME/ev3dev/ev3dev-lang-cpp
RUN mkdir build 
RUN cd build && CXXFLAGS="-fno-strict-aliasing" cmake .. -DEV3DEV_PLATFORM=EV3 -DCMAKE_INSTALL_PREFIX=/opt/ev3dds
RUN cd build && cmake --build . --target all -j6 && sudo make install
