#!/bin/bash -v
#
#  Build OZONE system disk from scratch
#
#    Input files:
#
#	$ROOT/bin86-0.14.5.tar.gz
#	$ROOT/binutils-2.12.tar.gz
#	$ROOT/gcc-2.7.2.tar.gz
#	$ROOT/gcc-2.95.3.tar.gz
#	$ROOT/glibc-2.0.6.tar.gz
#
#	$ROOT/step_by_step.bash (this file)
#	$ROOT/ozone/includes/
#	$ROOT/ozone/linux/
#	$ROOT/ozone/patches/
#	$ROOT/ozone/sources/
#	$ROOT/ozone/startup/
#	$ROOT/ozone/tests/
#
set -e
ROOT=`pwd`	# get what the root directory is
date
export PATH=.:$PATH
#
# Step 1 - create misc directories
#
rm -rf ozone/objects
rm -rf ozone/binaries
rm -rf ozone/libraries
rm -rf linux_utils
rm -rf bin86
rm -rf binutils
rm -rf gcc ## /binaries gcc/libraries gcc/sources
rm -rf glibc
#
mkdir ozone/objects
mkdir ozone/binaries
mkdir ozone/libraries
mkdir linux_utils
mkdir bin86
mkdir bin86/binaries
mkdir bin86/sources
mkdir binutils
mkdir binutils/binaries
mkdir binutils/libraries
mkdir binutils/sources
mkdir gcc
mkdir gcc/binaries
mkdir gcc/libraries
mkdir gcc/sources
mkdir glibc
##mkdir glibc/libraries
mkdir glibc/sources
#
# Step 2 - Unpack 'borrowed' original sources
#
cd gcc/sources
tar xzf $ROOT/gcc-2.7.2.tar.gz
tar xzf $ROOT/gcc-2.95.3.tar.gz
find . -exec chmod u+w {} \;
cd ../..
cd binutils/sources
tar xzf $ROOT/binutils-2.12.tar.gz
cd ../..
cd glibc/sources
tar xzf $ROOT/glibc-2.0.6.tar.gz
cd ../..
cd bin86/sources
tar xzf $ROOT/bin86-0.14.5.tar.gz
cd ../..
#
# Step 3 - create assembler
#
cd $ROOT/binutils/sources
tar xzvf $ROOT/ozone/patches/my_patch_binutils-2_12.tgz
cd binutils-2.12
configure --prefix=$ROOT/linux_utils
make
make install
##??cp gas/as-new $ROOT/linux_utils/as
#
# Step 4 - create compiler
#   I use gcc 2.7.2's preprocessor (cpp)
#   I use gcc 2.95.3's manager (gcc) and compiler (cc1)
#     2.95.3's prepreocessor is a mess and it didn't work
#
cd $ROOT/gcc/sources
tar xzvf $ROOT/ozone/patches/my_patch_gcc-2_7_2.tgz
tar xzvf $ROOT/ozone/patches/my_patch_gcc-2_95_3.tgz
cd gcc-2.7.2
configure --prefix=$ROOT/linux_utils --host i586-unknown-linux
make cccp
cp cccp $ROOT/linux_utils/cpp0
cd ../gcc-2.95.3
configure --prefix=$ROOT/linux_utils ## --host i586-unknown-linux
make
make install
##??cp gcc/xgcc $ROOT/linux_utils/gcc
##??cp gcc/cc1 $ROOT/linux_utils/cc1
#
cd $ROOT/linux_utils/bin
ln -s gcc xgcc
#
cd $ROOT/gcc
ln -s sources/gcc-2.95.3/gcc/include includes
cd $ROOT/glibc
ln -s /usr/include includes
#
# Step 5 - create ozone files needed by linux utilities
#
savepath=$PATH
export PATH=$ROOT/linux_utils/bin:$PATH
linupath=$PATH
#
cd $ROOT/ozone/sources
chmod 750 ozmake_486 *.bash
./ozmake_486 ../objects/oz_loader_486.r
#
# Step 6 - create linux utilities (elfconv, loader)
#   Used by OZONE build routines to create OZONE images
#
cd $ROOT/ozone/linux
chmod 750 ozmake
./ozmake
#
# Step 7 - create ozone binary files (boot, loader, kernel)
#   This is the main os build step
#
cd $ROOT/ozone/sources
./ozmake_486
#
# Step 8 - create ozone runtime includes
#
##cd libraries
##ln -s ../sources/glibc-2.0.6/libc.a libc.a
##cd $ROOT/gcc
##ln -s sources/gcc-2.7.2/include includes
cd $ROOT/gcc/libraries
ln -s ../sources/gcc-2.95.3/gcc/libgcc.a libgcc.a
##cd $ROOT/ozone/includes
##ln -s oz_crtl_fio.h stdio.h
#
# Step 8-1/2 - rebuild glibc library for ozone
#
##cd $ROOT/glibc/sources/glibc-2.0.6
##configure --prefix=$ROOT/linux_utils ## --host i586-unknown-linux --disable-sanity-checks
##make
#
# Step 9 - build ozone utilities ar, as, ld
#
cd $ROOT/binutils/sources/binutils-2.12/bfd
rm -f *.o *.a
make -f makebfd.ozone
#
cd ../libiberty
rm -f *.o *.a
make -f makeiberty.ozone
#
cd ../intl
rm -f *.o *.a
make -f makeintl.ozone
#
cd ../binutils
rm -f *.o
make -f makear.ozone
cp ar.oz ../../../binaries/
#
cd ../gas
rm -f *.o
make -f makegas.ozone
cp as.oz ../../../binaries/
#
cd ../ld
rm -f *.o
make -f makeld.ozone
cp ld.oz ../../../binaries/
#
# Step 10 - build ozone gcc, cpp0, cc1
#
cd $ROOT/gcc/sources/gcc-2.7.2
rm *.o
make -f makegcc.ozone cpp.oz
mv ../../binaries/cpp.oz ../../binaries/cpp0.oz
#
cd ../gcc-2.95.3/libiberty
rm -f *.o *.a
make -f makeiberty.ozone
cd ../gcc
rm *.o
make -f makegcc.ozone gcc.oz cc1.oz
cp gcc.oz ../../../binaries/
cp cc1.oz ../../../binaries/
#
# Step 11 - build ozone as86, ld86
#
cd $ROOT/bin86/sources/bin86/as
cp ../../../../ozone/patches/makeas86.ozone .
make -f makeas86.ozone
#
cd ../ld
cp ../../../../ozone/patches/makeld86.ozone .
cp ../../../../ozone/patches/ld86_execv.c .
make -f makeld86.ozone
#
# Step 12 - build EDT
#
cd $ROOT/ozone/edt
rm -f *.o
export PATH=$savepath
make		# build for linux
export PATH=$linupath
./ozmake	# build for ozone
#
# Step 13 - make disk images
#
cd $ROOT/ozone/sources
makeall
cd ..
#
# All done
#
ls -l
date
