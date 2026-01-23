#!/bin/sh

ROOT_VER=6.28.12
GEANT4_VERSION=10.6.1
CMAKE_VERSION=3.27.7
GCC_VERSION=12.2.0
FIFEUTILS_VERSION=3.7.4
EXPAT_VERSION=2.5.0
IFDH_VERSION=2.8.0

echo "Setting up spack ... "
source /cvmfs/larsoft.opensciencegrid.org/spack-packages/setup-env.sh
spack load gcc@${GCC_VERSION}; if [ $? -eq 0 ] ; then echo "gcc ${GCC_VERSION} loaded: location - $(spack location -i gcc@${GCC_VERSION})"; else echo "Failed to load gcc ${GCC_VERSION}"; exit 1; fi
spack load cmake@${CMAKE_VERSION}; if [ $? -eq 0 ] ; then echo "cmake ${CMAKE_VERSION} loaded: location - $(spack location -i cmake@${CMAKE_VERSION})"; else echo "Failed to load cmake ${CMAKE_VERSION}"; exit 1; fi
spack load expat@${EXPAT_VERSION}; if [ $? -eq 0 ] ; then echo "expat ${EXPAT_VERSION} loaded: location - $(spack location -i expat@${EXPAT_VERSION})"; else echo "Failed to load expat ${EXPAT_VERSION}"; exit 1; fi
spack load root@${ROOT_VER}; if [ $? -eq 0 ] ; then echo "root ${ROOT_VER} loaded: location - $(spack location -i root@${ROOT_VER})"; else echo "Failed to load root ${ROOT_VER}"; exit 1; fi
spack load geant4@${GEANT4_VERSION}; if [ $? -eq 0 ] ; then echo "geant4 ${GEANT4_VERSION} loaded: location - $(spack location -i geant4@${GEANT4_VERSION})"; else echo "Failed to load geant4 ${GEANT4_VERSION}"; exit 1; fi
spack load ifdhc@${IFDH_VERSION}; if [ $? -eq 0 ] ; then echo "ifdhc ${IFDH_VERSION} loaded: location - $(spack location -i ifdhc@${IFDH_VERSION})"; else echo "Failed to load ifdhc ${IFDH_VERSION}"; exit 1; fi
