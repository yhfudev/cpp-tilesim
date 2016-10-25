#!/bin/bash
# Name:        makemsw.sh
# Purpose:     cross compile script for compiling windows' software in Linux
# Author:      Yunhui Fu <yhfudev@gmail.com>
# Created:     2009-03-01
# Modified by: Yunhui Fu, 2011-01-15
# RCS-ID:      $Id: $
# Copyright:   (c) 2009,2011 Yunhui Fu
# Licence:     GPL licence version 3
##############################################################################

# 预先要安装的程序：
# wget bzr mercurial git-core
# bison flex patch make autoconf automake libtool scons yasm cmake intltool build-essential
# make wxwidgets gdb zlib freetype libpng libxml2 libiconv freeglut

# Set to 1 to compile the GCC compiler, or 0
FLG_COMPILE_GCC=0
# Set to 1 to compile the libraries for the application, or 0
FLG_COMPILE_LIBS=1
# Set to 1 to download the packages of the software, or 0
FLG_DOWNLOAD=0
# Set to 1 to extract the packages of the software, or 0
FLG_EXTRACT=0

# Stop if any command fails
#set -e

EXEC_GREP=`which grep`
EXEC_AWK=`which awk`
EXEC_XARGS=`which xargs`
EXEC_BASENAME=`which basename`
EXEC_ECHO=`which echo`
EXEC_WGET=`which wget`
EXEC_INSTALL=`which install`
EXEC_CAT=`which cat`
EXEC_TAR=`which tar`
EXEC_MD5SUM=`which md5sum`

#CONF_BUILD=i386-redhat-linux
# HOST: the  i386 windows
# BUILD: the prefix of the compiler
# TARGET: the cpu. such as arm
#build就是你正在使用的机器,
#host就是你编译好的程序可以运行的平台,
#target就是你编译的程序可以处理的平台.
CONF_BUILD="`/usr/share/libtool/config/config.guess`"
CONF_HOST=i586-mingw32msvc #"`/usr/share/libtool/config/config.guess`"
CONF_TARGET=i586-mingw32msvc

if [ "a${CONF_HOST}" = "a" ]; then
CONF_PARAM_HOST="--target=${CONF_TARGET}"
CROSS_PREFIX=
else
CONF_PARAM_HOST="--build=${CONF_BUILD} --host=${CONF_HOST} --target=${CONF_TARGET}"
CROSS_PREFIX="${CONF_HOST}-"
fi
CROSS_AR="${CROSS_PREFIX}ar"
CROSS_NM="${CROSS_PREFIX}nm"
CROSS_CC="${CROSS_PREFIX}gcc"
CROSS_CXX="${CROSS_PREFIX}g++"
CROSS_RANLIB="${CROSS_PREFIX}ranlib"
CROSS_RC="${CROSS_PREFIX}rc"
CROSS_DLLTOOL="${CROSS_PREFIX}dlltool"
CROSS_LDSHARED="${CROSS_PREFIX}gcc"

# directories:
#   builds/xxx.            build the xxx from source
#   packages/xxx.tar.bz2   the source package files
#   sources/xxxx/          the source untared
#   install/               the installation directory
DN_TOP="`pwd`"
DN_BUILD="${DN_TOP}/builds"
DN_PACKAGES="${DN_TOP}/packages"
DN_SOURCE="${DN_TOP}/sources"
DN_INSTALL="${DN_TOP}/install"
DN_INSTALL_SYS="/usr/${CONF_TARGET}"
# addon tools dir
DN_ADDON="${DN_TOP}/addon"

# Log file name
FN_LOG="${DN_TOP}/log.txt"
log () {
    THEDATE="`date '+%Y-%m-%d %H:%M:%S'`"
    echo "${THEDATE} | $*" >> "${FN_LOG}"
}
log "================= Starting ==============="

FN_MD5TMP=`pwd`/md5sumalltmp
cat << EOF > ${FN_MD5TMP}
7df692e3186109cc00db6825b777201e  packages/libdbi/openjade-1.3.2.tar.gz
ca66db78d479cbfa727cf3245b5864ae  packages/libdbi/libdbi-0.8.3.tar.gz
f46fe0a04b76a4454ec27b7fcc84ec54  packages/libdbi/OpenSP-1.5.1.tar.gz
2e29ca610579438714ebb1e4010a0ece  packages/libxml2/libxml2-2.7.5.tar.gz
58a2bc6d39c0ba57823034d55d65d606  packages/libxml2/gettext-0.17.tar.gz
092fcd018d35da599e250990e9b64e6f  packages/igraph/igraph-0.5.2.tar.gz
9c2744f1aa72fe755adda33663aa3fad  packages/freetype/freetype-2.3.9.tar.gz
6e41dc85a7963896fda75cdaf5d84021  packages/opengl/glut-3.7.6-src.zip
22e03dc4038cd63f32c21eb60994892b  packages/opengl/MesaDemos-7.2.tar.bz2
f67daf93e12c4a459703bbf3e4004e31  packages/opengl/MesaGLUT-7.2.tar.bz2
6d16873bd876fbf4980a927cfbc496a1  packages/opengl/freeglut-2.4.0.tar.gz
b12d4340854b578d9725694315667687  packages/opengl/other/demos.zip
9b126bd205a3858cc438dfab4634601d  packages/opengl/other/cube.c
04d379292e023df0b0266825cb0dbde5  packages/opengl/MesaLib-7.2.tar.bz2
5b3cdf95c5c90322c964f43893b9d0bb  packages/wx/wxWidgets-2.9.0.tar.gz
c9da266b884fb8fa54df786dfaadbc7a  packages/compiler/gdb-6.8.tar.bz2
fc62e989cf31d015f31628609fc3757a  packages/compiler/glibc-2.9.tar.bz2
d449047b5761348ceec23739f5553e0b  packages/compiler/gcc-g++-4.4.1.tar.bz2
d19693308aa6b2052e14c071111df59f  packages/compiler/gcc-core-4.4.1.tar.bz2
17a52219dee5a76c1a9d9b0bfd337d66  packages/compiler/binutils-2.19.tar.bz2
927eaac3d44b22f31f9c83df82f26436  packages/compiler/gcc-4.4.1.tar.bz2
e67a98eef14212abfc265092e4683fbe  packages/crosscompile/busybox-1.15.1.tar.bz2
c8a3e25cb8e6cb15cea97636ce0369c0  packages/crosscompile/buildroot-2009.08.tar.bz2
df0632df30b3ed3e184babe796d96ae1  packages/crosscompile/libsdl/x86-mingw32-build.sh-0.0-20061107-1.tar.gz
46fa98eb920ae514147590e7532ddbb0  packages/crosscompile/crosstool-ng-1.4.2.tar.bz2
862e5dd259bbe51f5cef1c3e053f6043  packages/crosscompile/mingw_tdm/tdminstall-1.808.1.zip
109cfd7982d44982a88caaacee3d9751  packages/crosscompile/mingw_tdm/gcc-4.4.1-tdm-1-srcbase.zip
0dfa8765eee84dca467dd039eb122ddc  packages/zlib/zlib_1.2.3-6ubuntu4.diff.gz
903f33e2f38f409edfc7a84866378c3b  packages/zlib/mingw-zlib-1.2.3-3-src.tar.bz2
dee233bf288ee795ac96a98cc2e369b6  packages/zlib/zlib-1.2.3.tar.bz2
d12e82e398cb095d630efa0ab8178030  packages/zlib/libpng_1.2.27-2ubuntu2.diff.gz
a2f6808735bf404967f81519a967fb2a  packages/zlib/libpng-1.2.40.tar.gz
debc62758716a169df9f62e6ab2bc634  packages/zlib/zlib-1.2.3.tar.gz
f15fe752d8b7012aa5e59f83b88ccb1c  packages/crosscompile/busybox-1.18.1.tar.bz2
9499200fddad0dc3c5765ebafaf0c395  packages/crosscompile/buildroot-2010.11.tar.bz2
ad761f188408c4a53c8489f28fbef766  packages/crosscompile/crosstool-ng-1.9.2.tar.bz2
b1a8107f99b5d953e8418a5409462294  packages/opengl/freeglut-2.6.0-rc1.tar.gz
47963ece64fe5f793e154e238bc6c3c3  packages/igraph/igraph-0.5.4.tar.gz
c5f15407ef7b07ba854cd8c9b15b88ff  packages/libdbi/libdbi-0.8.4.tar.gz
cbf3d8be3e3516dcb12b751de822b48c  packages/libdbi/openjade-1.3.3-pre1.tar.gz
670b223c5d12cee40c9137be86b6c39b  packages/libdbi/OpenSP-1.5.2.tar.gz
4de79b323162a5a7652b65b608eca6cd  packages/libdbi/libdbi-drivers-0.8.3-1.tar.gz
6d30c693233b1464ef8983fedd8ccb22  packages/pthread/pthreads-w32-2-8-0-release.tar.gz
b3e2b6e2f1c3e0dffa1fd2a0f848b671  packages/freetype/freetype-2.4.4.tar.bz2
9821f1c61e43755866861485ff364e90  packages/compiler/gcc-g++-4.5.2.tar.bz2
870d76d46dcaba37c13d01dca47d1774  packages/compiler/glibc-linuxthreads-2.5.tar.bz2
aa9e36bec080452372bfba793428ee82  packages/compiler/gcc-core-4.5.2.tar.bz2
903fcfa547df2f453476800e0838fe52  packages/compiler/glibc-2.12.2.tar.bz2
d6559145853fbaaa0fd7556ed93bce9a  packages/compiler/gcc-4.5.2.tar.bz2
64260e6c56979ee750a01055f16091a5  packages/compiler/gdb-7.2.tar.bz2
c84c5acc9d266f1a7044b51c85a823f5  packages/compiler/binutils-2.21.tar.bz2
0966e19f03217db9e9076894b47e6601  packages/compiler/newlib-1.19.0.tar.gz
0df377025082cd93cccbca547f048011  packages/libxml2/mingw32-libxml2-static-build-compile-fix.patch
d73b75385b1aa40fc2bf74f84989bc0c  packages/libxml2/mingw32-libxml2-2.7.2-with-modules.patch
0a5eec8c74739a5513086f46eb960120  packages/libxml2/mingw32-libxml2.spec
8127a65e8c3b08856093099b52599c86  packages/libxml2/libxml2-2.7.8.tar.gz
c1cf4cf00447615be2794b7ae984b659  packages/libxml2/libxml2-gnome-bug-561340-fix.patch
d52a3e061032a1ed13856d42fc86f0fd  packages/libxml2/gettext-0.18.tar.gz
7ab33ebd26687c744a37264a330bbe9a  packages/libxml2/libiconv-1.13.1.tar.gz
c735eab2d659a96e5a594c9e8541ad63  packages/zlib/zlib-1.2.5.tar.gz
07e920837979eb6b75c8b80b513ba8da  packages/zlib/mingw-zlib-1.2.3-10-src.tar.bz2
e3ac7879d62ad166a6f0c7441390d12b  packages/zlib/libpng-1.2.44.tar.bz2
94f75fa41b7398e61f691091b14fd9ed  packages/zlib/libpng_1.2.44-1.debian.tar.bz2
1c5c355c0349471fd5e235ab293f73ff  packages/zlib/zlib_1.2.3.4.dfsg-3ubuntu1.debian.tar.gz
81c20d7b2ba31becb18e467dbe09be8f  packages/wx/wxWidgets-2.9.1.tar.bz2
EOF

# we declair a array to list all of the files to be downloaded, including the path of the package to be stored.
# the path and URL of the package are splited by ' '.
declare -A PKG_MAP
PKG_MAP=(
    ["binutils"]="compiler http://ftp.gnu.org/gnu/binutils/binutils-2.21.tar.bz2"
    ["gcc"]="compiler http://ftp.gnu.org/gnu/gcc/gcc-4.5.2/gcc-4.5.2.tar.bz2"
    ["gcc-core"]="compiler http://ftp.gnu.org/gnu/gcc/gcc-4.5.2/gcc-core-4.5.2.tar.bz2"
    ["gcc-g++"]="compiler http://ftp.gnu.org/gnu/gcc/gcc-4.5.2/gcc-g++-4.5.2.tar.bz2"
    #["gmplib"]="compiler ftp://gcc.gnu.org/pub/gcc/infrastructure/gmp-4.3.2.tar.bz2" # required by gcc 4.5
    ["gmplib"]="compiler ftp://ftp.gmplib.org/pub/gmp-4.3.2/gmp-4.3.2.tar.bz2" # required by gcc 4.5
    #["gmplib"]="compiler ftp://ftp.gmplib.org/pub/gmp-5.0.1/gmp-5.0.1.tar.bz2" # required by gcc 4.5
    #["mpfr"]="compiler ftp://gcc.gnu.org/pub/gcc/infrastructure/mpfr-2.4.2.tar.bz2" # required by gcc 4.5
    ["mpfr"]="compiler http://www.mpfr.org/mpfr-current/mpfr-3.0.0.tar.bz2" # required by gcc 4.5
    ["mpc"]="compiler ftp://gcc.gnu.org/pub/gcc/infrastructure/mpc-0.8.1.tar.gz" # required by gcc 4.5
    ["newlib"]="compiler ftp://sources.redhat.com/pub/newlib/newlib-1.19.0.tar.gz"

    ["expat"]="compiler http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fexpat%2Ffiles%2Fexpat%2F2.0.1%2F&ts=1295206826&use_mirror=cdnetworks-us-1"
    ["glibc"]="compiler http://ftp.gnu.org/gnu/glibc/glibc-2.12.2.tar.bz2"
    ["glibc-linuxthreads"]="compiler http://ftp.gnu.org/gnu/glibc/glibc-linuxthreads-2.5.tar.bz2"
    ["gdb"]="compiler http://ftp.gnu.org/gnu/gdb/gdb-7.2.tar.bz2"
    ["crosstool-ng"]="crosscompile http://ymorin.is-a-geek.org/download/crosstool-ng/crosstool-ng-1.9.2.tar.bz2"
    ["busybox"]="crosscompile http://busybox.net/downloads/busybox-1.18.1.tar.bz2"
    ["buildroot"]="crosscompile http://buildroot.uclibc.org/downloads/buildroot-2010.11.tar.bz2"

    ["pthreads-w32"]="pthread ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.tar.gz"

    ["zlib"]="zlib http://zlib.net/zlib-1.2.5.tar.gz"
    ["zlib-debian"]="zlib http://archive.ubuntu.com/ubuntu/pool/main/z/zlib/zlib_1.2.3.4.dfsg-3ubuntu1.debian.tar.gz"
    ["mingw-zlib"]="zlib http://ftp.uni-kl.de/pub/windows/cygwin/release/mingw/mingw-zlib/mingw-zlib-1.2.3-10-src.tar.bz2"
    ["libpng"]="zlib http://superb-sea2.dl.sourceforge.net/project/libpng/libpng12/1.2.44/libpng-1.2.44.tar.bz2"
    ["libpng-debian"]="zlib http://archive.ubuntu.com/ubuntu/pool/main/libp/libpng/libpng_1.2.44-1.debian.tar.bz2"

    ["gettext"]="libxml2 http://ftp.gnu.org/gnu/gettext/gettext-0.18.tar.gz"
    ["libiconv"]="libxml2 http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.13.1.tar.gz"
    ["libxml2"]="libxml2 http://xmlsoft.org/sources/libxml2-2.7.8.tar.gz"
    ["libxml2-gnome-bug-561340-fix.patch"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=libxml2-gnome-bug-561340-fix.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["mingw32-libxml2-static-build-compile-fix.patch"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=mingw32-libxml2-static-build-compile-fix.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["mingw32-libxml2-2.7.2-with-modules.patch"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=mingw32-libxml2-2.7.2-with-modules.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["mingw32-libxml2.spec"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=mingw32-libxml2.spec;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["igraph"]="igraph http://downloads.sourceforge.net/project/igraph/C%20library/0.5.4/igraph-0.5.4.tar.gz?r=http%3A%2F%2Figraph.sourceforge.net%2Fdownload.html&ts=1295146552&use_mirror=cdnetworks-us-1"

    ["wxWidgets"]="wx http://downloads.sourceforge.net/project/wxwindows/2.9.1/wxWidgets-2.9.1.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fwxwindows%2Ffiles%2F2.9.1%2F&ts=1295106250&use_mirror=superb-sea2"
    ["freetype"]="freetype http://nongnu.askapache.com/freetype/freetype-2.4.4.tar.bz2"

    ["OpenSP"]="libdbi http://downloads.sourceforge.net/project/openjade/opensp/1.5.2/OpenSP-1.5.2.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fopenjade%2Ffiles%2Fopensp%2F1.5.2%2F&ts=1295106370&use_mirror=softlayer"
    ["openjade"]="libdbi http://downloads.sourceforge.net/project/openjade/openjade/1.3.3/openjade-1.3.3-pre1.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fopenjade%2Ffiles%2Fopenjade%2F1.3.3%2F&ts=1295106425&use_mirror=superb-sea2"
    ["libdbi"]="libdbi http://downloads.sourceforge.net/project/libdbi/libdbi/libdbi-0.8.4/libdbi-0.8.4.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Flibdbi%2Ffiles%2Flibdbi%2Flibdbi-0.8.4%2F&ts=1295106470&use_mirror=surfnet"
    ["libdbi-drivers"]="libdbi http://downloads.sourceforge.net/project/libdbi-drivers/libdbi-drivers/libdbi-drivers-0.8.3-1/libdbi-drivers-0.8.3-1.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Flibdbi-drivers%2Ffiles%2Flibdbi-drivers%2Flibdbi-drivers-0.8.3-1%2F&ts=1295106510&use_mirror=voxel"

    ["freeglut"]="opengl http://downloads.sourceforge.net/project/freeglut/freeglut/2.6.0/freeglut-2.6.0-rc1.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Ffreeglut%2Ffiles%2Ffreeglut%2F2.6.0%2F&ts=1295106547&use_mirror=iweb"
  )

# The old package files
declare -A PKG_MAP_OLD
PKG_MAP_OLD=(
    ["binutils"]="compiler http://ftp.gnu.org/gnu/binutils/binutils-2.19.tar.bz2"

    ["gcc"]="compiler http://ftp.gnu.org/gnu/gcc/gcc-4.4.1/gcc-4.4.1.tar.bz2"
    ["gcc-core"]="compiler http://ftp.gnu.org/gnu/gcc/gcc-4.4.1/gcc-core-4.4.1.tar.bz2"
    ["gcc-g++"]="compiler http://ftp.gnu.org/gnu/gcc/gcc-4.4.1/gcc-g++-4.4.1.tar.bz2"

    ["glibc"]="compiler http://ftp.gnu.org/gnu/glibc/glibc-2.9.tar.bz2"
    ["glibc-linuxthreads"]="compiler http://ftp.gnu.org/gnu/glibc/glibc-linuxthreads-2.5.tar.bz2"

    ["gdb"]="compiler http://ftp.gnu.org/gnu/gdb/gdb-6.8.tar.bz2"
    ["expat"]="compiler http://downloads.sourceforge.net/project/expat/expat/2.0.1/expat-2.0.1.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fexpat%2Ffiles%2Fexpat%2F2.0.1%2F&ts=1295206826&use_mirror=cdnetworks-us-1"

    ["crosstool-ng"]="crosscompile http://ymorin.is-a-geek.org/download/crosstool-ng/crosstool-ng-1.4.2.tar.bz2"
    ["busybox"]="crosscompile http://busybox.net/downloads/busybox-1.15.1.tar.bz2"
    ["buildroot"]="crosscompile http://buildroot.uclibc.org/downloads/buildroot-2009.08.tar.bz2"
    ["pthreads-w32"]="pthread ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.tar.gz"

    ["zlib"]="zlib http://www.zlib.net/zlib-1.2.3.tar.gz"
    ["zlib-debian"]="zlib http://archive.ubuntu.com/ubuntu/pool/main/z/zlib/zlib_1.2.3-6ubuntu4.diff.gz"
    ["mingw-zlib"]="zlib http://cygwin.elite-systems.org/release/mingw/mingw-zlib/mingw-zlib-1.2.3-3-src.tar.bz2"
    ["libpng"]="zlib http://downloads.sourceforge.net/project/libpng/00-libpng-stable/1.2.40/libpng-1.2.40.tar.gz?use_mirror=softlayer"
    ["libpng-debian"]="zlib http://archive.ubuntu.com/ubuntu/pool/main/libp/libpng/libpng_1.2.27-2ubuntu2.diff.gz"

    ["gettext"]="libxml2 http://ftp.gnu.org/gnu/gettext/gettext-0.17.tar.gz"
    ["libiconv"]="libxml2 http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.13.1.tar.gz"
    ["libxml2"]="libxml2 http://xmlsoft.org/sources/libxml2-2.7.5.tar.gz"
    ["libxml2-gnome-bug-561340-fix.patch"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=libxml2-gnome-bug-561340-fix.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["mingw32-libxml2-static-build-compile-fix.patch"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=mingw32-libxml2-static-build-compile-fix.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["mingw32-libxml2-2.7.2-with-modules.patch"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=mingw32-libxml2-2.7.2-with-modules.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["mingw32-libxml2.spec"]="libxml2 http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=mingw32-libxml2.spec;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9"
    ["igraph"]="igraph http://softlayer.dl.sourceforge.net/project/igraph/igraph/0.5.2/igraph-0.5.2.tar.gz"
    ["wxWidgets"]="wx   http://downloads.sourceforge.net/project/wxwindows/wxAll/2.9.0/wxWidgets-2.9.0.tar.gz?use_mirror=softlayer"
    ["freetype"]="freetype http://nongnu.askapache.com/freetype/freetype-2.3.9.tar.gz"

    ["OpenSP"]="libdbi http://prdownloads.sourceforge.net/openjade/OpenSP-1.5.1.tar.gz"
    ["openjade"]="libdbi http://prdownloads.sourceforge.net/openjade/openjade-1.3.2.tar.gz"
    ["libdbi"]="libdbi http://downloads.sourceforge.net/project/libdbi/libdbi/libdbi-0.8.3/libdbi-0.8.3.tar.gz?use_mirror=softlayer"
    ["libdbi-drivers"]="libdbi http://downloads.sourceforge.net/project/libdbi-drivers/libdbi-drivers/libdbi-drivers-0.8.3-1/libdbi-drivers-0.8.3-1.tar.gz?use_mirror=softlayer"

    ["freeglut"]="opengl http://downloads.sourceforge.net/project/freeglut/freeglut/2.6.0/freeglut-2.6.0-rc1.tar.gz?use_mirror=softlayer"

  )

get_partname () {
    PARAM_PARTNAME=`${EXEC_ECHO} "${PKG_MAP[$1]}" | ${EXEC_AWK} '{print $1}'`
    echo "${PARAM_PARTNAME}"
}

get_filename () {
    PARAM_URL_DOWN="${PKG_MAP[$1]}"
    PARAM_URL_DOWN=`${EXEC_ECHO} ${PARAM_URL_DOWN} | ${EXEC_AWK} '{print $2}'`
    if [ "`${EXEC_ECHO} ${PARAM_URL_DOWN} | ${EXEC_GREP} 'http://pkgs.fedoraproject.org/gitweb'`" = "" ]; then
        PARAM_NAME_SRC="`${EXEC_ECHO} ${PARAM_URL_DOWN} | ${EXEC_AWK} -F? '{print $1}' | ${EXEC_XARGS} ${EXEC_BASENAME}`"
    else
        #http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=libxml2-gnome-bug-561340-fix.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9
        PARAM_NAME_SRC="`${EXEC_ECHO} ${PARAM_URL_DOWN} | ${EXEC_AWK} -F? '{print $2}' | ${EXEC_AWK} -F\; '{print $3}' | ${EXEC_AWK} -F= '{print $2}'`"
    fi
    echo "${PARAM_NAME_SRC}"
}

get_dirname () {
    PARAM_NAME_SRC=`get_filename "$1"`
    DN_SRC=`echo "${PARAM_NAME_SRC}" | awk -F. '{print $1}' `
    for k in `ls "${DN_SOURCE}" | grep "${DN_SRC}"`; do
        TMP=`echo "${PARAM_NAME_SRC}" | grep $k`
        if [ ! "${TMP}" = "" ]; then
            DN_SRC=$k
            break
        fi
    done
    echo "${DN_SRC}"
}

extract_file () {
  ARG_FN=$1
  ARG_DN=$2
  if [ -f "${ARG_FN}" ]; then
    case "${ARG_FN}" in
    *.tar.gz)
      if [ "${ARG_DN}" = "" ]; then
        tar -xzf "${ARG_FN}"
      else
        tar -C "${ARG_DN}" -xzf "${ARG_FN}"
      fi
      ;;
    *.gz)
      if [ "${ARG_DN}" = "" ]; then
        gunzip "${ARG_FN}"
      else
        cd "${ARG_DN}"; gunzip "${ARG_FN}"; cd -
      fi
      ;;
    *.tgz)
      if [ "${ARG_DN}" = "" ]; then
        tar -xzf "${ARG_FN}"
      else
        tar -C "${ARG_DN}" -xzf "${ARG_FN}"
      fi
      ;;
    *.tar.bz2)
      if [ "${ARG_DN}" = "" ]; then
        tar -xjf "${ARG_FN}"
      else
        tar -C "${ARG_DN}" -xjf "${ARG_FN}"
      fi
      ;;
    *.bz2)
      if [ "${ARG_DN}" = "" ]; then
        bunzip2 ${ARG_FN}
      else
        cd "${ARG_DN}"; bunzip2 "${ARG_FN}"; cd -
      fi
      ;;
    *.tbz)
      if [ "${ARG_DN}" = "" ]; then
        tar -xjf "${ARG_FN}"
      else
        tar -C "${ARG_DN}" -xjf "${ARG_FN}"
      fi
      ;;
    *)
      ;;
    esac
  else
    echo "Not found file: ${ARG_FN}"
    return 1
  fi
  return 0;
}

# Download file from internet
# $1: the directory for place the downloaded package
# $2: the url of the software
download_file_path () {
    PARAM_DN_SUBDIR=$1
    shift
    PARAM_URL_DOWN=$1
    shift
    if [ "`${EXEC_ECHO} ${PARAM_URL_DOWN} | ${EXEC_GREP} 'http://pkgs.fedoraproject.org/gitweb'`" = "" ]; then
        PARAM_NAME_SRC="`${EXEC_ECHO} ${PARAM_URL_DOWN} | ${EXEC_AWK} -F? '{print $1}' | ${EXEC_XARGS} ${EXEC_BASENAME}`"
    else
        #http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=libxml2-gnome-bug-561340-fix.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9
        PARAM_NAME_SRC="`${EXEC_ECHO} ${PARAM_URL_DOWN} | ${EXEC_AWK} -F? '{print $2}' | ${EXEC_AWK} -F\; '{print $3}' | ${EXEC_AWK} -F= '{print $2}'`"
    fi

    ${EXEC_INSTALL} -d "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/"
    if [ "${FLG_DOWNLOAD}" = "1" ]; then
        FLG_ERR=1
        if [ -f "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" ]; then
#echo "DBG: PARAM_NAME_SRC=${PARAM_NAME_SRC}"
#echo "DBG: content of ${FN_MD5TMP}: " ; ${EXEC_CAT} "${FN_MD5TMP}"

            MD5SUMVAL0=`${EXEC_CAT} "${FN_MD5TMP}" | ${EXEC_GREP} "${PARAM_NAME_SRC}" | ${EXEC_AWK} '{print $1}'`
            if [ ! "${MD5SUMVAL0}" = "" ]; then
                MD5SUMVAL1=`${EXEC_MD5SUM} ${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC} | ${EXEC_AWK} '{print $1}'`
                if [ "${MD5SUMVAL0}" = "${MD5SUMVAL1}" ]; then
                    FLG_ERR=0
                else
                    echo "md5sum error: record=${MD5SUMVAL0}; calculated=${MD5SUMVAL1}"
                fi
            else
                echo "Unable get the md5sum of file ${PARAM_NAME_SRC}"
            fi
        else
            echo "Not found file: ${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}"
        fi
        if [ "${FLG_ERR}" = "0" ]; then
            echo "The package was downloaded correctly, skiping ${PARAM_NAME_SRC} ..."
        else
            # error in file
            echo "Error in file ${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}, so re-download it"
            #rm -f ${DN_SRCS}/${FNDOWN}

            echo "download ${PARAM_NAME_SRC} ..."
            #echo "${EXEC_WGET} -c -O ${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC} ${PARAM_URL_DOWN}"
            ${EXEC_WGET} -c -O "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" ${PARAM_URL_DOWN}
        fi
    fi
    if [ "${FLG_EXTRACT}" = "1" ]; then
        echo "extract ${PARAM_NAME_SRC} ..."
        ${EXEC_INSTALL} -d "${DN_SOURCE}"

        #${EXEC_TAR} -C "${DN_SOURCE}" -xf "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}"
        extract_file "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" "${DN_SOURCE}"
        if [ $? = 1 ]; then
            echo "ERROR in extracting file: ${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}"
        fi
    fi
}

# Download a software part from internet.
# $1: the name of the software, such as gcc, gcc-core etc.
# the function will get the URL from pre-configured global PKG_MAP
download_file () {
    PKG_PATH="${PKG_MAP[$1]}"
    PKG_URL=`${EXEC_ECHO} ${PKG_PATH} | ${EXEC_AWK} '{print $2}'`
    PKG_PATH=`${EXEC_ECHO} ${PKG_PATH} | ${EXEC_AWK} '{print $1}'`

    if [ "`${EXEC_ECHO} ${PKG_URL} | ${EXEC_GREP} 'http://pkgs.fedoraproject.org/gitweb'`" = "" ]; then
        PKG_FILENAME="`${EXEC_ECHO} ${PKG_URL} | ${EXEC_AWK} -F? '{print $1}' | ${EXEC_XARGS} ${EXEC_BASENAME}`"
    else
        #http://pkgs.fedoraproject.org/gitweb/?p=mingw32-libxml2.git;a=blob_plain;f=libxml2-gnome-bug-561340-fix.patch;hb=3ca44fff2d6acb82881d8ae91b65614a3f10cec9
        PKG_FILENAME="`${EXEC_ECHO} ${PKG_URL} | ${EXEC_AWK} -F? '{print $2}' | ${EXEC_AWK} -F\; '{print $3}' | ${EXEC_AWK} -F= '{print $2}'`"
    fi
    #echo "k=${KEY}; p=${PKG_PATH}; f=${PKG_FILENAME}; url=${PKG_URL}"

    download_file_path "${PKG_PATH}" "${PKG_URL}"
}

# get the source from GIT
# $1: the path for the source
# $1: git url
# Example: fetch_git "/home/usr/source/libcmsis-git" "git://git.open-bldc.org/libcmsis.git"
fetch_git () {
    PARAM_PATH=$1
    shift
    PARAM_URL=$1
    shift
    if [ ! -d "${PARAM_PATH}" ]; then
        # pull the source
        git clone "${PARAM_URL}" "${PARAM_PATH}"
    fi
    if [ ! -d "${PARAM_PATH}/.git/" ]; then
        echo "Fatal: unable to find the git dir!"
        return 1
    fi
    cd "${PARAM_PATH}"
    git pull
    #git archive --format=tar --prefix=libcmsis-${LIBCMSIS}/ ${LIBCMSIS} | bzip2 --stdout > ../libcmsis-${LIBCMSIS}.tar.bz2
    cd -
    return 0
}

# get the source from BZR
# $1: the path for the source
# $1: bzr url
# Example: fetch_bzr "igraph" "lp:igraph"
fetch_bzr () {
    PARAM_PATH=$1
    shift
    PARAM_URL=$1
    shift
    if [ ! -d "${PARAM_PATH}" ]; then
        # pull the source
        bzr branch "${PARAM_URL}" "${PARAM_PATH}"
    fi
    if [ ! -d "${PARAM_PATH}/.bzr/" ]; then
        echo "Fatal: unable to find the igraph bzr!"
        return 1
    fi
    cd "${PARAM_PATH}"
    bzr merge
    cd -
    return 0
}

md5sum_packages_stdin () {
    while :; do
        read TMPFN
        if [ ! $? = 0 ]; then
            break
        fi
        if [ "${TMPFN}" = "" ]; then
            continue
        fi
        # skip SVN's database
        echo "${TMPFN}" | grep "/.svn/" > /dev/null 2>&1
        if [ $? = 0 ]; then
            continue
        fi
        # skip BZR's database
        echo "${TMPFN}" | grep "/.bzr/" > /dev/null 2>&1
        if [ $? = 0 ]; then
            continue
        fi
        if [ -f "${TMPFN}" ]; then
            md5sum "${TMPFN}"
        fi
    done
}

md5sum_packages () {
    find "`basename ${DN_PACKAGES}`" > tmpfilelisttmp
    md5sum_packages_stdin < tmpfilelisttmp > tmpfilelisttmp2
    cat tmpfilelisttmp2
    rm -f tmpfilelisttmp tmpfilelisttmp2
}

check_result () {
    RET=$?
    PARAM_MESSAGE="$1"
    if [ ${RET} = 0 ]; then
        echo "=========== ${PARAM_MESSAGE} successful ! ==========="
        log "${PARAM_MESSAGE} successful."
    else
        echo "!!!!!!!!!!! ${PARAM_MESSAGE} FAILED ! !!!!!!!!!!!"
        log "${PARAM_MESSAGE} FAILED!"
        exit 1
    fi
}

build_prepare () {
    PARAM_NAME_SRC="$1"
    shift
    PARAM_SUBDN_BUILD="$1"
    shift
    PARAM_ADDON=$@

    install -d "${DN_BUILD}/${PARAM_SUBDN_BUILD}"
    cd "${DN_BUILD}/${PARAM_SUBDN_BUILD}"
    echo "***** ${PARAM_SUBDN_BUILD}/configure --prefix=${DN_INSTALL} ${CONF_PARAM_HOST} ${PARAM_ADDON}"
echo PATH=$PATH:${DN_INSTALL}/bin:${DN_INSTALL}/lib \
      PKG_CONFIG_PATH="${DN_INSTALL}/lib/pkgconfig/" \
      CFLAGS="-I${DN_INSTALL}/include" \
      CPPFLAGS="-I${DN_INSTALL}/include" \
      LDFLAGS="-L${DN_INSTALL}/lib" \
      AR=${CROSS_AR} \
      NM=${CROSS_NM} \
      CC=${CROSS_CC} \
      CXX=${CROSS_CXX} \
      RANLIB=${CROSS_RANLIB} \
      LDSHARED=${CROSS_LDSHARED} \
      "${DN_SOURCE}/${PARAM_NAME_SRC}/configure" --prefix="${DN_INSTALL}" ${CONF_PARAM_HOST} ${PARAM_ADDON}
    # environment setup & configure
    PATH=$PATH:${DN_INSTALL}/bin:${DN_INSTALL}/lib \
      PKG_CONFIG_PATH="${DN_INSTALL}/lib/pkgconfig/" \
      CFLAGS="-I${DN_INSTALL}/include" \
      CPPFLAGS="-I${DN_INSTALL}/include" \
      LDFLAGS="-L${DN_INSTALL}/lib" \
      AR=${CROSS_AR} \
      NM=${CROSS_NM} \
      CC=${CROSS_CC} \
      CXX=${CROSS_CXX} \
      RANLIB=${CROSS_RANLIB} \
      LDSHARED=${CROSS_LDSHARED} \
      "${DN_SOURCE}/${PARAM_NAME_SRC}/configure" --prefix="${DN_INSTALL}" ${CONF_PARAM_HOST} ${PARAM_ADDON}
    check_result "configure ${PARAM_SUBDN_BUILD}"
}

install -d "${DN_BUILD}"
install -d "${DN_PACKAGES}"
install -d "${DN_SOURCE}"
install -d "${DN_INSTALL}"
install -d "${DN_INSTALL}/bin/"
install -d "${DN_INSTALL}/lib/"
install -d "${DN_INSTALL}/include/"
install -d "${DN_INSTALL}/share/man/man3"
install -d "${DN_ADDON}"

# test if the shell is in mingw or cygwin
cat > test.c << EOF
#include <stdio.h>
int
main ()
{
#ifdef __CYGWIN__
printf ("cygwin\n");
#elif defined(__MINGW32__)
printf ("mingw32\n");
#else
printf ("unknown\n");
#endif
}
EOF
gcc -o test test.c
TMPTYPE=`./test`
rm test test.c
FLG_CYGWIN=0
if [ ${TMPTYPE} = "cygwin" ]; then
  FLG_CYGWIN=1
fi

#sudo apt-get update
#sudo apt-get upgrade
sudo apt-get install wget bzr bison flex patch make autoconf automake libtool

if [ ${FLG_DOWNLOAD} = 1 -o ${FLG_EXTRACT} = 1 ]; then

if [ 0 = 1 ]; then
echo "the number of the items to be downloaded is ${#PKG_MAP[@]}"
for KEY in "${!PKG_MAP[@]}"; do
    download_file "${KEY}" #"${PKG_MAP[$KEY]}"
done
fi # 0

#echo "Calculating MD5 ..."
#md5sum_packages

if [ ${FLG_COMPILE_GCC} = 1 ]; then
log "Starting download software group GCC ..."
download_file binutils

download_file gcc
download_file gcc-core
download_file gcc-g++
download_file newlib
download_file gmplib
download_file mpfr
download_file mpc

TMPDN=`get_dirname "mpc"`
cd "${DN_SOURCE}/${TMPDN}"
TMPDN=`get_partname "mpc"`
patch -p1 -i "${DN_PACKAGES}/${TMPDN}/mpc-0.8.1-constant.patch"
cd -

#download_file glibc
#download_file glibc-linuxthreads

download_file expat
download_file gdb

#download_file crosstool-ng
#download_file busybox
#download_file buildroot
log "Done download software group GCC"
fi

#if [ 0 = 1 ]; then # DEBUG comment
if [ ${FLG_COMPILE_LIBS} = 1 ]; then
log "Starting download software group LIB ..."
download_file pthreads-w32

download_file zlib
download_file zlib-debian
download_file mingw-zlib
download_file libpng
download_file libpng-debian

PKG_PATH="${PKG_MAP[zlib]}"
PARAM_DN_SUBDIR=`${EXEC_ECHO} ${PKG_PATH} | ${EXEC_AWK} '{print $1}'`
TMPDN=`get_dirname "zlib"`

echo "patch zlib ..."
PARAM_NAME_SRC=`get_filename "mingw-zlib"`
extract_file "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" "${DN_SOURCE}/${TMPDN}"
cd "${DN_SOURCE}/${TMPDN}"
#patch -p2 -i "${DN_SOURCE}/mingw-zlib-1.2.3-10.src.patch"
#patch -p2 -i "${DN_SOURCE}/mingw-zlib-1.2.3-10.cygwin.patch"
if [ ${FLG_CYGWIN} = 1 ]; then
  ${DN_SOURCE}/mingw-zlib-1.2.3-10.cygport
fi
cd -

PARAM_NAME_SRC=`get_filename "zlib-debian"`
#cd "${DN_SOURCE}/${TMPDN}"; gzip -c -d "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" | patch -p1 ; cd -
extract_file  "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" "${DN_SOURCE}/${TMPDN}"

echo "patch libpng ..."
TMPDN=`get_dirname "libpng"`
PARAM_NAME_SRC=`get_filename "libpng-debian"`
#cd "${DN_SOURCE}/${TMPDN}"; gzip -c -d "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" | patch -p1 ; cd -
extract_file  "${DN_PACKAGES}/${PARAM_DN_SUBDIR}/${PARAM_NAME_SRC}" "${DN_SOURCE}/${TMPDN}"

#download_file gettext
download_file libiconv
download_file libxml2
download_file libxml2-gnome-bug-561340-fix.patch
download_file mingw32-libxml2-static-build-compile-fix.patch
download_file mingw32-libxml2-2.7.2-with-modules.patch
download_file mingw32-libxml2.spec
PARAM_NAME_SRC=`get_dirname libxml2`
cd "${DN_SOURCE}/${PARAM_NAME_SRC}/"
TMPDN=`get_partname "libxml2"`
patch -p0 -i "${DN_PACKAGES}/${TMPDN}/libxml2-gnome-bug-561340-fix.patch"
patch -p1 -i "${DN_PACKAGES}/${TMPDN}/mingw32-libxml2-2.7.2-with-modules.patch"
patch -p0 -i "${DN_PACKAGES}/${TMPDN}/mingw32-libxml2-static-build-compile-fix.patch"
libtoolize && autoconf && automake

#download_file igraph
DN_IGRAPH_BZR="${DN_PACKAGES}/igraph/igraph-bzr/"
fetch_bzr "${DN_IGRAPH_BZR}" "lp:igraph"

rm -rf "${DN_SOURCE}/igraph-bzr/"
cp -r "${DN_IGRAPH_BZR}" "${DN_SOURCE}/igraph-bzr/"

cd "${DN_SOURCE}/igraph-bzr/"
TMPDN=`get_partname "igraph"`
#patch -p0 -i "${DN_PACKAGES}/${TMPDN}/igraph-bzr-200910-debianmakefile.patch"
echo "paching complex.patch ..."
patch -p1 -i "${DN_PACKAGES}/${TMPDN}/igraph-bzr-201101-complex.patch"
echo "bootstrap ..."
./bootstrap.sh
if [ ${FLG_CYGWIN} = 1 ]; then
echo "update the lex files ..."
cp "${DN_PACKAGES}/igraph/bzr_flex_out/foreign-ncol-lexer.c" "${DN_SOURCE}/igraph-bzr/foreign-ncol-lexer.c"
touch "${DN_SOURCE}/igraph-bzr/foreign-ncol-lexer.c"
cp "${DN_PACKAGES}/igraph/bzr_flex_out/foreign-lgl-lexer.c" "${DN_SOURCE}/igraph-bzr/foreign-lgl-lexer.c"
touch "${DN_SOURCE}/igraph-bzr/foreign-lgl-lexer.c"
cp "${DN_PACKAGES}/igraph/bzr_flex_out/foreign-pajek-lexer.c" "${DN_SOURCE}/igraph-bzr/foreign-pajek-lexer.c"
touch "${DN_SOURCE}/igraph-bzr/foreign-pajek-lexer.c"
cp "${DN_PACKAGES}/igraph/bzr_flex_out/foreign-gml-lexer.c" "${DN_SOURCE}/igraph-bzr/foreign-gml-lexer.c"
touch "${DN_SOURCE}/igraph-bzr/foreign-gml-lexer.c"
cp "${DN_PACKAGES}/igraph/bzr_flex_out/foreign-dl-lexer.c" "${DN_SOURCE}/igraph-bzr/foreign-dl-lexer.c"
touch "${DN_SOURCE}/igraph-bzr/foreign-dl-lexer.c"
fi

download_file wxWidgets
download_file freetype

download_file OpenSP
download_file openjade
download_file libdbi
download_file libdbi-drivers

download_file freeglut
PARAM_NAME_SRC=`get_dirname freeglut`
cd "${DN_SOURCE}/${PARAM_NAME_SRC}/"
TMPDN=`get_partname "freeglut"`
patch -p1 -i "${DN_PACKAGES}/${TMPDN}/freeglut-2.6.0-mingw.patch"
patch -p1 -i "${DN_PACKAGES}/${TMPDN}/freeglut-2.6.0-configure.patch"
./autogen.sh
fi
log "Done download software group LIB"
fi

if [ ${FLG_COMPILE_GCC} = 0 ]; then
# 如果不是自己编译编译器，则需要查找系统中的，同时修改 AR, CC 等的前缀。
dpkg --get-selections | grep -v deinstall | grep mingw32
if [ ! a$? = a0 ]; then
    echo "not found mingw32 in your system. please install it:"
    sudo apt-get install mingw32 mingw32-binutils mingw32-runtime
fi
dpkg --get-selections | grep -v deinstall | grep mingw32

CROSS_PREFIX="${CONF_HOST}-"
which ${CROSS_PREFIX}gcc
if [ ! $? = 0 ]; then
    CONF_HOST="mingw32"
    CONF_BUILD="`/usr/share/libtool/config/config.guess`"
    CROSS_PREFIX="${CONF_HOST}-"
    which ${CROSS_PREFIX}gcc
    if [ ! $? = 0 ]; then
        CONF_HOST=
        CONF_BUILD=
        CROSS_PREFIX=
    fi
fi
CROSS_RANLIB="${CROSS_PREFIX}ranlib"
which ${CROSS_RANLIB}
if [ ! $? = 0 ]; then
    which ranlib
    if [ ! $? = 0 ]; then
        echo "unable to find ranlib!"
        exit 1
    fi
    CROSS_RANLIB="`which ranlib`"
    CONF_HOST=
    CONF_BUILD=
fi
CROSS_RC="${CROSS_PREFIX}windres"
which ${CROSS_RC}
if [ ! $? = 0 ]; then
    which windres
    if [ ! $? = 0 ]; then
        echo "unable to find windres!"
        exit 1
    fi
    CROSS_RC="`which windres`"
fi
CROSS_DLLTOOL="${CROSS_PREFIX}dlltool"
which ${CROSS_DLLTOOL}
if [ ! $? = 0 ]; then
    which dlltool
    if [ ! $? = 0 ]; then
        echo "unable to find dlltool!"
        exit 1
    fi
    CROSS_DLLTOOL="`which dlltool`"
fi
if [ a${CONF_HOST} = a ]; then
    CONF_PARAM_HOST=""
    CROSS_AR="ar"
    CROSS_NM="nm"
    CROSS_CC="gcc"
    CROSS_CXX="g++"
    CROSS_RANLIB="ranlib"
    CROSS_LDSHARED="gcc"

    which gcc

else
    CONF_PARAM_HOST="--build=${CONF_BUILD} --host=${CONF_HOST}"
    CROSS_AR="${CROSS_PREFIX}ar"
    CROSS_NM="${CROSS_PREFIX}nm"
    CROSS_CC="${CROSS_PREFIX}gcc"
    CROSS_CXX="${CROSS_PREFIX}g++"
    CROSS_RANLIB="${CROSS_PREFIX}ranlib"
    CROSS_LDSHARED="${CROSS_PREFIX}gcc"

    which ${CROSS_PREFIX}gcc
fi
check_result "Finding compiler"

#report:
echo "CONF_PARAM_HOST=${CONF_PARAM_HOST}"
echo "AR=${CROSS_AR}"
echo "NM=${CROSS_NM}"
echo "CC=${CROSS_CC}"
echo "CXX=${CROSS_CXX}"
echo "RANLIB=${CROSS_RANLIB}"
echo "LDSHARED=${CROSS_LDSHARED}"
echo "RC=${CROSS_RC}"
echo "DLLTOOL=${CROSS_DLLTOOL}"
echo ""

else
# compile GCC by myself?

if [ 1 = 1 ]; then # DEBUG comment

log "Compiling binutils ..."
# Binutils
PARAM_NAME_SRC=`get_dirname binutils`
# --disable-nls tells binutils not to include native language support, only English
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --enable-interwork --enable-multilib --disable-nls --disable-shared --disable-threads --with-gcc --with-gnu-as --with-gnu-ld
make all install
check_result "make ${PARAM_NAME_SRC}"

log "Compiling gmplib ..."
PARAM_NAME_SRC=`get_dirname gmplib`
# ABI (Application Binary Interface) 
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" "--prefix=${DN_ADDON}" ABI="32"
#make all check
#check_result "make ${PARAM_NAME_SRC}"
make install
check_result "make install for ${PARAM_NAME_SRC}"

log "Compiling mpfr ..."
PARAM_NAME_SRC=`get_dirname mpfr`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" "--prefix=${DN_ADDON}" "--with-gmp=${DN_ADDON}" --disable-shared
make all install
check_result "make ${PARAM_NAME_SRC}"

log "Compiling mpc ..."
PARAM_NAME_SRC=`get_dirname mpc`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" "--prefix=${DN_ADDON}" "--with-gmp=${DN_ADDON}"--disable-shared
make all install
check_result "make ${PARAM_NAME_SRC}"

# Minimal Gcc
log "Compiling mini gcc ..."
PARAM_NAME_SRC=`get_dirname gcc`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --disable-nls --disable-shared --disable-threads --disable-libssp --disable-libstdcxx-pch --disable-libmudflap --disable-libgomp --enable-languages=c --enable-interwork --enable-multilib --without-headers --with-gcc --with-gnu-ld --with-gnu-as --with-dwarf2 --with-newlib "--with-gmp=${DN_ADDON}" "--with-mpfr=${DN_ADDON}" "--with-mpc=${DN_ADDON}"
#--enable-interwork --enable-multilib -with-newlib --with-headers=../newlib-$NEWLIB_VER/newlib/libc/include
make all-gcc install-gcc
check_result "make ${PARAM_NAME_SRC}"

# One of C libraries:
# 1) Glibc (http://www.gnu.org/software/libc/)
# 2) newlib (http://sources.redhat.com/newlib/)
# 3) PDPCLIB (http://cvs.sourceforge.net/viewcvs.py/pdos/pdos/pdpclib/)
# 4) PDCLib (http://pdclib.rootdirectory.de/)
if [ 0 = 1 ]; then
# Glibc
log "Compiling glibc ..."
#PARAM_NAME_SRC=`get_dirname glibc-linuxthreads`
PARAM_NAME_SRC=`get_dirname glibc`
# install glibc-linuxthreads
touch /etc/ld.so.conf   #用该命令来修正编译时警告缺少/etc/ld.so.conf文件
#patch -Np1 -i ../glibc-2.3.2-sscanf-1.patch #该补丁在http://www.linuxfromscratch.org/patches/downloads/glibc/
#../glibc-2.3.2/configure --prefix=/usr --disable-profile --enable-add-ons --libexecdir=/usr/bin --with-headers=/usr/include #其中/usr/include下最好是linux-libc-headers包下的干净头文件
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" ${CONF_PARAM_HOST} --with-binutils="${DN_INSTALL}/bin" \
  --libdir "${DN_INSTALL}/${CONF_TARGET}/lib"
#  --enable-add-ons=linuxthreads --with-headers=${Path_to_your_powerpc_linux_kernel_tree}/linuxppc_2_4_devel/include
#make
#make check
#make install
#make localedata/install-locales
make all install
fi

# newlib
log "Compiling newlib ..."
PARAM_NAME_SRC=`get_dirname newlib`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" \
                         --enable-interwork --enable-multilib \
                         --with-gnu-as \
                         --with-gnu-ld \
                         --disable-nls \
                         --disable-werror \
                         --disable-newlib-supplied-syscalls

make all install
check_result "make ${PARAM_NAME_SRC}"

fi # DEBUG comment

# Full Gcc
log "Compiling Full gcc ..."
PARAM_NAME_SRC=`get_dirname gcc`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --enable-shared --enable-threads --enable-languages=c,c++ --enable-c99 --enable-long-long --with-newlib "--with-gmp=${DN_ADDON}" "--with-mpfr=${DN_ADDON}" "--with-mpc=${DN_ADDON}"
make all install
check_result "make ${PARAM_NAME_SRC}"

# expat
log "Compiling expat ..."
PARAM_NAME_SRC=`get_dirname expat`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" "--prefix=${DN_ADDON}" --disable-shared
make all install
check_result "make install for ${PARAM_NAME_SRC}"

# gdb
log "Compiling gdb ..."
PARAM_NAME_SRC=`get_dirname gdb`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --enable-interwork --enable-multilib --disable-werror --with-libexpat-prefix="${DN_ADDON}"
make all install
check_result "make ${PARAM_NAME_SRC}"

# strip

log "Strip files ..."
for f in \
	${DN_INSTALL}/bin/* \
	${DN_INSTALL}/${CONF_TARGET}/bin/* \
	${DN_INSTALL}/libexec/gcc/${CONF_TARGET}/$GCC_VER/*
do
	strip $f
done

fi

if [ ${FLG_COMPILE_LIBS} = 1 ]; then
#if [ 0 = 1 ]; then

# zlib
log "Compiling zlib ..."
PARAM_NAME_SRC=`get_dirname zlib`
ln -s "${DN_SOURCE}/${PARAM_NAME_SRC}" "${DN_BUILD}/${PARAM_NAME_SRC}"
cd "${DN_BUILD}/${PARAM_NAME_SRC}"
#build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --shared
##CC=${CROSS_CC} AR=${CROSS_AR} NM=${CROSS_NM} RANLIB=${CROSS_RANLIB} ${DN_SOURCE}/${PARAM_NAME_SRC}/configure --prefix=${DN_INSTALL_SYS}
CC=${CROSS_CC} AR=${CROSS_AR} NM=${CROSS_NM} RANLIB=${CROSS_RANLIB} \
  LDSHARED="${CROSS_CC} -shared -Wl,-soname,libz.so.1" \
  "${DN_SOURCE}/${PARAM_NAME_SRC}/configure" --shared --prefix="${DN_INSTALL}"
make AR="${CROSS_AR} r" NM=${CROSS_NM} RANLIB=${CROSS_RANLIB}
##${CROSS_RANLIB} "${DN_BUILD}/${PARAM_NAME_SRC}/libz.a"
check_result "make ${PARAM_NAME_SRC}"
make install prefix="${DN_INSTALL}"
check_result "install ${PARAM_NAME_SRC}"

if [ ! ${FLG_CYGWIN} = 1 ]; then
# pthread-w32
log "Compiling pthread-w32 ..."
#PARAM_NAME_SRC=pthreads-w32-2-8-0-release
PARAM_NAME_SRC=`get_dirname pthreads-w32`
ln -s "${DN_SOURCE}/${PARAM_NAME_SRC}" "${DN_BUILD}/${PARAM_NAME_SRC}"
cd "${DN_BUILD}/${PARAM_NAME_SRC}"
make clean GC-inlined AR=${CROSS_AR} NM=${CROSS_NM} CC=${CROSS_CC} RC=${CROSS_RC} DLLTOOL=${CROSS_DLLTOOL} RANLIB=${CROSS_RANLIB}

check_result "make ${PARAM_NAME_SRC}"
cp pthread*.dll "${DN_INSTALL}/bin"
cp pthread*.lib "${DN_INSTALL}/lib"
cp libpthr*.a   "${DN_INSTALL}/lib"
cp pthread.h    "${DN_INSTALL}/include"
cp sched.h      "${DN_INSTALL}/include"
cp semaphore.h  "${DN_INSTALL}/include"
fi

# libpng
#log "Compiling libpng ..."
#PARAM_NAME_SRC=`get_dirname libpng`
#build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --with-gnu-ld --enable-shared
#make install RANLIB=${CROSS_RANLIB}
#check_result "make ${PARAM_NAME_SRC}"

# iconv
log "Compiling iconv ..."
PARAM_NAME_SRC=`get_dirname libiconv`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}"
make install
check_result "make ${PARAM_NAME_SRC}"

# libxml2
log "Compiling libxml2 ..."
PARAM_NAME_SRC=`get_dirname libxml2`
# refer: http://cvs.fedoraproject.org/viewvc/rpms/mingw32-libxml2/devel/mingw32-libxml2.spec?revision=1.15&view=markup
CONF_PARAM="--with-gnu-ld --without-python --with-modules --with-zlib --with-iconv --with-threads=yes --enable-debug=no"
#--with-threads=win32
# build shared:
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}-shared" ${CONF_PARAM} --disable-static --enable-shared
# First install all the files belonging to the shared build
make install
check_result "make ${PARAM_NAME_SRC} (shared)"

# build static:
log "Compiling static libxml2 ..."
PARAM_NAME_SRC=`get_dirname libxml2`
# Install all the files from the static build in a seperate folder
# and move the static libraries to the right location
rm -rf "${DN_INSTALL}/build_static"
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}-static" ${CONF_PARAM} --enable-static --disable-shared CFLAGS="-DLIBXML_STATIC_FOR_DLL $CFLAGS"
make DESTDIR="${DN_INSTALL}/build_static" install
check_result "make ${PARAM_NAME_SRC} (static)"
echo mv "${DN_INSTALL}/build_static/${DN_INSTALL}/lib/libxml2.a" "${DN_INSTALL}/lib/"
mv "${DN_INSTALL}/build_static/${DN_INSTALL}/lib/libxml2.a" "${DN_INSTALL}/lib/"
# Manually merge the libtool files
sed -i s/"old_library=''"/"old_library='libxml2.a'"/ "${DN_INSTALL}/lib/libxml2.la"
# Drop the folder which was temporary used for installing the static bits
rm -rf "${DN_INSTALL}/build_static"

# gettext
#log "Compiling gettext ..."
#PARAM_NAME_SRC=`get_dirname gettext`
#build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --with-libiconv-prefix="${DN_INSTALL}" --with-libxml2-prefix="${DN_INSTALL}"
#make install
#check_result "make ${PARAM_NAME_SRC}"
#fi

# igraph
log "Compiling igraph ..."
build_prepare igraph-bzr igraph-bzr
make install
check_result "make igraph-bzr"

# freetype2
log "Compiling freetype2 ..."
PARAM_NAME_SRC=`get_dirname freetype`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}"
make all install
check_result "make ${PARAM_NAME_SRC}"

if [ ! ${FLG_CYGWIN} = 1 ]; then
# opengl
log "Compiling freeglut ..."
PARAM_NAME_SRC=`get_dirname freeglut`
# mingw contains libglaux.a,libglu32.a,libglut.a,libglut32.a,libopengl32.a. But it's lack of glut.h, so replace it.
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}"
make install
check_result "make ${PARAM_NAME_SRC}"
fi

#wxWidgets
log "Compiling wxWidgets ..."
PARAM_NAME_SRC=`get_dirname wxWidgets`
build_prepare "${PARAM_NAME_SRC}" "${PARAM_NAME_SRC}" --with-opengl \
  --with-zlib="builtin" --with-libpng="builtin" --with-libjpeg --with-libtiff \
  --enable-unicode --with-libiconv --with-libiconv-prefix="${DN_INSTALL}" \
  --with-msw

make install
check_result "make ${PARAM_NAME_SRC}"

if [ ! ${FLG_CYGWIN} = 1 ]; then

lnkwxlib () {
    DIR=$1
    shift
    NAME=$1
    shift
    ln -s ${DIR}/${NAME}-${CONF_TARGET}.dll.a ${DIR}/${NAME}.a
}

lnkwxlib "${DN_INSTALL}/lib" libwx_baseu-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_baseu_net-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_baseu_xml-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_adv-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_aui-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_core-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_gl-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_html-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_media-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_propgrid-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_qa-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_richtext-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_stc-2.9
lnkwxlib "${DN_INSTALL}/lib" libwx_mswu_xrc-2.9
fi

fi

# compile tilesim3d
log "Compiling application tilesim3d ..."
ln -s "${DN_TOP}/.." "${DN_SOURCE}/tilesim"
make -C "${DN_SOURCE}/tilesim" distclean
make -C "${DN_BUILD}/tilesim" distclean
#build_prepare tilesim tilesim --enable-debug
build_prepare tilesim tilesim
make
check_result "make tilesim"

if [ ! ${FLG_CYGWIN} = 1 ]; then
# pack the tilesim
log "Packing the application tilesim ..."
install -d "${DN_TOP}/tilesim-release"
cp "${DN_INSTALL}/bin/libglut-0.dll"           ${DN_TOP}/tilesim-release
cp "${DN_INSTALL}/bin/libiconv-2.dll"          ${DN_TOP}/tilesim-release
cp "${DN_INSTALL}/bin/pthreadGC2.dll"          ${DN_TOP}/tilesim-release
cp "${DN_INSTALL}/lib/wxbase290u_gcc_custom.dll" ${DN_TOP}/tilesim-release
cp "${DN_INSTALL}/lib/wxmsw290u_core_gcc_custom.dll" ${DN_TOP}/tilesim-release
cp "${DN_INSTALL}/lib/wxmsw290u_gl_gcc_custom.dll" ${DN_TOP}/tilesim-release
cp "${DN_PACKAGES}/mingwm10.dll"               ${DN_TOP}/tilesim-release
cp "${DN_BUILD}/tilesim/src/uiglut/tsglut.exe" ${DN_TOP}/tilesim-release
cp "${DN_BUILD}/tilesim/src/uiwx/tswx.exe"     ${DN_TOP}/tilesim-release
cp "${DN_BUILD}/tilesim/src/utils/bintilecreator.exe" ${DN_TOP}/tilesim-release
cp "${DN_BUILD}/tilesim/src/utils/cubecreator.exe"    ${DN_TOP}/tilesim-release
cp "${DN_BUILD}/tilesim/src/utils/squarecreator.exe"  ${DN_TOP}/tilesim-release
log "Done packing the application tilesim."
else

fi

rm -f ${FN_MD5TMP}

log "Done!"
