cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

add_executable(app_main main.c)

target_sources(app_main
     PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            ${PARENT_DIR}/tusb_src
            ${PARENT_DIR}/file_processing
            ${PARENT_DIR}/hardware_processing
            ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            ${PARENT_DIR}/tusb_src/msc_app.h
            ${PARENT_DIR}/tusb_src/embedded_cli.h
            ${PARENT_DIR}/file_processing/file_processing.h
            ${PARENT_DIR}/hardware_processing/hardware_processing.h
)

## target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(app_main PRIVATE hardware_sync file_processing hardware_structs)
target_compile_definitions(app_main  PRIVATE PICO_NO_RAM_VECTOR_TABLE=0 PICO_USE_SW_SPIN_LOCKS=0)
pico_enable_stdio_uart(app_main 0)

pico_add_extra_outputs(app_main)

