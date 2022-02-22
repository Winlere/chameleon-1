#!/usr/bin/env bash
#
# @file build_macosx.sh
#
# @copyright 2020-2022 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                      Univ. Bordeaux. All rights reserved.
#
# @version 1.0.0
# @author Florent Pruvost
# @date 2020-03-03
#

# setup openblas
. ~/.bash_profile

cd build-starpu
cmake .. -DCMAKE_INSTALL_PREFIX=$PWD/../install -DCHAMELEON_PREC_D=OFF -DCHAMELEON_PREC_C=OFF -DCHAMELEON_PREC_Z=OFF -DBLA_PREFER_PKGCONFIG=ON -DBUILD_SHARED_LIBS=ON
make -j5
make install
