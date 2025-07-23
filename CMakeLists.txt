cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

project(fty-warranty
  VERSION 1.0.0
  DESCRIPTION "fty-warranty"
)

find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR}/fty-cmake)

##############################################################################################################

etn_target(exe biostimer-warranty-metric
    SOURCES
        src/*.cc
    USES_PRIVATE
        cxxtools
        tntdb
        fty_common_db
        fty_common_logging
        fty_shm
        fty_proto
    TARGET_DESTINATION
        "${CMAKE_INSTALL_LIBEXECDIR}"
)

# Agent config
etn_configure_file(
    resources/biostimer-warranty-metric.cfg.in
    TARGET      biostimer-warranty-metric
    DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}"
)

# systemd unit
etn_configure_file(
    resources/biostimer-warranty-metric.service.in
    TARGET      biostimer-warranty-metric
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/systemd/system/"
)

# systemd tmpfiles
etn_configure_file(
    resources/biostimer-warranty-metric.timer.in
    TARGET      biostimer-warranty-metric
    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/systemd/system/"
)
