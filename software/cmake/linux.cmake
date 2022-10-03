###############################################################################
# Linux Specific Project Variables
###############################################################################

###############################################################################
# Compiler Settings
###############################################################################

###############################################################################
# Compiling Flags
###############################################################################
set(C_FLAGS "-Wall \
-Og \
-g")

set(C_DEFS "-D__linux__")

set(CMAKE_C_FLAGS "${C_FLAGS} ${C_DEFS}")
set(CMAKE_CXX_FLAGS "${C_FLAGS} ${C_DEFS}")
set(CMAKE_ASM_FLAGS "${C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "-lc \
-lm \
-Wl,-Map=${PROJECT_NAME}.map,--cref")