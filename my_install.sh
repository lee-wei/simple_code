#!/bin/sh

./configure --host=mipsel-linux --prefix="$PWD/release" --without-ada --enable-termcap --with-shared --disable-werror
make
make install
