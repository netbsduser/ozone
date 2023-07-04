#!/bin/bash -v
export PATH=`pwd`/linux_utils/486:$PATH
#
#  relink bin86 stuff
#
cd bin86/sources/bin86/as
rm -f as86.oz
make -f makeas86.ozone
cd ../ld
rm -f ld86.oz
make -f makeld86.ozone
cd ../../../..
#
#  relink binutils stuff
#
cd binutils/sources/binutils-2.12/binutils
rm -f ar.r ar.oz
make -f makear.ozone
cp ar.oz ../../../binaries/
cd ../gas
rm -f as.r as.oz
make -f makegas.ozone
cp as.oz ../../../binaries/
cd ../ld
rm -f ld.r ld.oz
make -f makeld.ozone
cp ld.oz ../../../binaries/
cd ../../../..
#
#  relink gcc stuff
#
cd gcc/sources/gcc-2.7.2
rm -f cpp.r cpp.oz
make -f makegcc.ozone cpp.oz
cp cpp.oz ../../binaries/cpp0.oz
cd ../../..
#
cd gcc/sources/gcc-2.95.3/gcc
rm -f gcc.r cc1.r gcc.oz cc1.oz
make -f makegcc.ozone gcc.oz cc1.oz
cp gcc.oz ../../../binaries/
cp cc1.oz ../../../binaries/
cd ../../../..
#
#  relink edt
#
cd ozone/edt
rm -f edt.r ../binaries/edt.oz
ozmake
cd ../..
#
#  list out
#
ls -l bin86/binaries
ls -l binutils/binaries
ls -l gcc/binaries
ls -l ozone/binaries/edt.oz
