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
##target_compile_definitions(app_main PUBLIC CFG_TUH_ENABLED=1)



target_link_libraries(app_main PUBLIC file_processing)
target_compile_definitions(app_main PUBLIC CFG_TUH_ENABLED=1 CFG_TUH_LOG=0)

pico_add_extra_outputs(app_main)
