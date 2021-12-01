#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/SimpleProtocol.o \
	${OBJECTDIR}/src/stdafx.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath,/home/ChessComposer/Debug -L/home/ChessComposer/Debug -lprotocolframework

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk /home/ChessComposer/${CND_CONF}/libSimpleProtocol.so

/home/ChessComposer/${CND_CONF}/libSimpleProtocol.so: /home/ChessComposer/Debug/libprotocolframework.so

/home/ChessComposer/${CND_CONF}/libSimpleProtocol.so: ${OBJECTFILES}
	${MKDIR} -p /home/ChessComposer/${CND_CONF}
	${LINK.cc} -o /home/ChessComposer/${CND_CONF}/libSimpleProtocol.so ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/src/SimpleProtocol.o: src/SimpleProtocol.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../ProtocolFramework/src -std=c++11 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/SimpleProtocol.o src/SimpleProtocol.cpp

${OBJECTDIR}/src/stdafx.o: src/stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -I../ProtocolFramework/src -std=c++11 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/stdafx.o src/stdafx.cpp

# Subprojects
.build-subprojects:
	cd ../ProtocolFramework && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} /home/ChessComposer/${CND_CONF}/libSimpleProtocol.so

# Subprojects
.clean-subprojects:
	cd ../ProtocolFramework && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
