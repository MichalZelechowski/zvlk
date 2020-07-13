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
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Camera.o \
	${OBJECTDIR}/Device.o \
	${OBJECTDIR}/Engine.o \
	${OBJECTDIR}/FragmentShader.o \
	${OBJECTDIR}/Frame.o \
	${OBJECTDIR}/Light.o \
	${OBJECTDIR}/Material.o \
	${OBJECTDIR}/Model.o \
	${OBJECTDIR}/Shader.o \
	${OBJECTDIR}/Texture.o \
	${OBJECTDIR}/TransformationMatrices.o \
	${OBJECTDIR}/UniformBuffer.o \
	${OBJECTDIR}/VertexShader.o \
	${OBJECTDIR}/Vulkan.o \
	${OBJECTDIR}/Window.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-fno-strict-aliasing
CXXFLAGS=-fno-strict-aliasing

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`pkg-config --libs vulkan` `pkg-config --libs glfw3` `pkg-config --libs libzip` `pkg-config --libs glm`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vulkanstarter

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vulkanstarter: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	g++ -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/vulkanstarter ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/Camera.o: Camera.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Camera.o Camera.cpp

${OBJECTDIR}/Device.o: Device.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Device.o Device.cpp

${OBJECTDIR}/Engine.o: Engine.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Engine.o Engine.cpp

${OBJECTDIR}/FragmentShader.o: FragmentShader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FragmentShader.o FragmentShader.cpp

${OBJECTDIR}/Frame.o: Frame.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Frame.o Frame.cpp

${OBJECTDIR}/Light.o: Light.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Light.o Light.cpp

${OBJECTDIR}/Material.o: Material.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Material.o Material.cpp

${OBJECTDIR}/Model.o: Model.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Model.o Model.cpp

${OBJECTDIR}/Shader.o: Shader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Shader.o Shader.cpp

${OBJECTDIR}/Texture.o: Texture.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Texture.o Texture.cpp

${OBJECTDIR}/TransformationMatrices.o: TransformationMatrices.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TransformationMatrices.o TransformationMatrices.cpp

${OBJECTDIR}/UniformBuffer.o: UniformBuffer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UniformBuffer.o UniformBuffer.cpp

${OBJECTDIR}/VertexShader.o: VertexShader.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/VertexShader.o VertexShader.cpp

${OBJECTDIR}/Vulkan.o: Vulkan.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Vulkan.o Vulkan.cpp

${OBJECTDIR}/Window.o: Window.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Window.o Window.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -w -s -DNDEBUG -Iinclude `pkg-config --cflags vulkan` `pkg-config --cflags glfw3` `pkg-config --cflags libzip` `pkg-config --cflags glm` -std=c++17  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
