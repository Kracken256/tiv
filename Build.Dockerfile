FROM ubuntu:20.04   

# Install dependencies
RUN apt update
RUN apt install -y libjpeg-dev cmake make clang

VOLUME /app

WORKDIR /app

RUN echo "#!/bin/sh" > /var/lib/build.sh
RUN echo "mkdir -p /app/build" >> /var/lib/build.sh 
RUN echo "cd /app/build" >> /var/lib/build.sh
RUN echo "cmake .." >> /var/lib/build.sh
RUN echo "make" >> /var/lib/build.sh

CMD ["/bin/sh", "/var/lib/build.sh"]