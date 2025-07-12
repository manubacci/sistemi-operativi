#
# data structure for process control block and semaphone and namespace
#

add_library(pcb pcb.c)

add_library(ash ash.c)
add_library(ns ns.c)
target_link_libraries(ash PRIVATE pcb)

# add current directories to include path
set_target_properties(
    pcb ash ns
    PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
)