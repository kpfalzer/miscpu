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
FC=
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug_linux32
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/miscpu.o \
	${OBJECTDIR}/perfmon.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m32
CXXFLAGS=-m32

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../../../xyzzy/dist/Debug_Linux32/GNU-Linux-x86/libxyzzy.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-Debug_linux32.mk dist/Debug_linux32/GNU-Linux-x86/miscpu

dist/Debug_linux32/GNU-Linux-x86/miscpu: ../../../../xyzzy/dist/Debug_Linux32/GNU-Linux-x86/libxyzzy.a

dist/Debug_linux32/GNU-Linux-x86/miscpu: ${OBJECTFILES}
	${MKDIR} -p dist/Debug_linux32/GNU-Linux-x86
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/miscpu ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/miscpu.o: miscpu.cxx 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -DDEBUG -DM32 -I../../../../xyzzy/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/miscpu.o miscpu.cxx

${OBJECTDIR}/perfmon.o: perfmon.cxx 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -DDEBUG -DM32 -I../../../../xyzzy/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/perfmon.o perfmon.cxx

${OBJECTDIR}/main.o: main.cxx 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -DDEBUG -DM32 -I../../../../xyzzy/src -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cxx

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug_linux32
	${RM} dist/Debug_linux32/GNU-Linux-x86/miscpu

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
