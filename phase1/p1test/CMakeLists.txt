#
# p1test
#

# l'unico file eseguibile: p1test.c
add_executable(kernel p1test.c)
target_link_libraries(kernel crtso libumps pcb ash ns)  #link crtso, libumps, pcb, ash, ns to kernel

# Indica di mettere il file generato nella radice della cartella in cui si esegue make
set_target_properties(
    kernel
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)