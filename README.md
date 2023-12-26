# ev3dds
Lego Mindstorm EV3 DDS agent
============================

To build the docker image:

   docker build -t myev3cc .

To run the image in container:

   docker run -it -v /home/$USER/projects/ev3dds/:/src -w /src myev3cc

Preparing target system - based on ev3dev-stretch image
-------------------------------------------------------

- Update apt sources

   sudo sh -c "echo 'deb http://archive.debian.org/debian stretch main contrib non-free' > /etc/apt/sources.list"
   sudo sh -c "echo 'deb-src http://archive.debian.org/debian stretch main contrib non-free' >> /etc/apt/sources.list"
   sudo sh -c "echo 'deb http://archive.ev3dev.org/debian stretch main' >> /etc/apt/sources.list"
   sudo sh -c "echo 'deb-src http://archive.ev3dev.org/debian stretch main' >> /etc/apt/sources.list"
   sudo apt update

- Install additional libs

   sudo apt install libssl-dev

- Copy built libraries and programs to EV3 

 ~ In the docker container, pack the installed software

  cd /src
  tar czf ev3dds.tgz /opt/ev3dds

 ~ Copy the file to EV3 with scp and unpack in /opt
 ~ Add the line 
     /opt/ev3dds/lib 
   to /etc/ld.so.conf
 ~ Run 

    sudo ldconfig
  
Running the agent
-----------------

- Copy the program ev3ddsnonde to the EV3 with scp.
- Log into the machine with ssh and start the program. 
