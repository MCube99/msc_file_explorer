
set(hardware_processing_Sources ${CMAKE_CURRENT_SOURCE_DIR}/spi_processing.c)
set(hardware_processing_Headers ${CMAKE_CURRENT_SOURCE_DIR}/hardware_processing.h})

add_library(hardware_processing STATIC ${hardware_processing_Sources})
target_include_directories(hardware_processing
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(hardware_processing PRIVATE hardware_spi hardware_gpio hardware_irq PUBLIC fatfs_lib)
 

# add_library(string_processing spi_processing.c hardware_processing.h)
# target_link_directories(string_processing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# target_link_libraries(string_processing PUBLIC )
