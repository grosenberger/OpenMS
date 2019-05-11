FROM grosenberger/openms_library:latest

# build TOPP & UTILS
WORKDIR /code/OpenMS_build

RUN make TOPP UTILS && make install
WORKDIR /
