set(examples
    logger_example
    loggerconf_example
)
include_directories(
    ${PROJECT_SOURCE_DIR}/src
)
foreach(example IN LISTS examples)
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} ${PROJECT_NAME}_static)
endforeach()
