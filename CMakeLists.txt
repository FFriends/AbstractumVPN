cmake_minimum_required(VERSION 3.25.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT AbstractumVPN)

# Base version of the product. The release workflow exports
# ABSTRACTUM_BUILD_NUMBER - the two-digit ISO year followed by the ISO week,
# e.g. 2631 - and it becomes the fourth component, so each weekly release
# compares as strictly newer than the one before it. Update checks rely on that
# comparison; a frozen version means no update is ever offered.
#
# Without the variable - local builds, the nightly build check - the fourth
# component stays 0, so a development build never claims to be newer than a
# published release. It must stay a number either way: the Windows resource
# below expands VERSION_TWEAK into FILEVERSION, which needs four of them.
set(AMNEZIAVPN_VERSION 5.0.2)
if(DEFINED ENV{ABSTRACTUM_BUILD_NUMBER} AND NOT "$ENV{ABSTRACTUM_BUILD_NUMBER}" STREQUAL "")
    set(AMNEZIAVPN_VERSION ${AMNEZIAVPN_VERSION}.$ENV{ABSTRACTUM_BUILD_NUMBER})
else()
    set(AMNEZIAVPN_VERSION ${AMNEZIAVPN_VERSION}.0)
endif()

set(QT_CREATOR_SKIP_PACKAGE_MANAGER_SETUP ON CACHE BOOL "" FORCE)
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES
    ${CMAKE_SOURCE_DIR}/cmake/platform_settings.cmake
    ${CMAKE_SOURCE_DIR}/cmake/recipes_bootstrap.cmake
    ${CMAKE_SOURCE_DIR}/cmake/conan_provider.cmake
    CACHE STRING "" FORCE)

project(${PROJECT} VERSION ${AMNEZIAVPN_VERSION}
        DESCRIPTION "AbstractumVPN"
        HOMEPAGE_URL "https://github.com/FFriends/AbstractumVPN"
)

# trigger conan to kick off `conan install` globally
find_package(OpenSSL REQUIRED)
if (PREBUILTS_ONLY)
    return()
endif()

string(TIMESTAMP CURRENT_DATE "%Y-%m-%d")
set(RELEASE_DATE "${CURRENT_DATE}")

set(APP_MAJOR_VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})
set(APP_ANDROID_VERSION_CODE 2141)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(MZ_PLATFORM_NAME "linux")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(MZ_PLATFORM_NAME "windows")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(MZ_PLATFORM_NAME "macos")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    set(MZ_PLATFORM_NAME "android")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(MZ_PLATFORM_NAME "ios")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(MZ_PLATFORM_NAME "wasm")
endif()

set(QT_BUILD_TOOLS_WHEN_CROSS_COMPILING ON)

if(APPLE AND NOT IOS)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(AMN_PF_RULE_IDENTITY "user { root }")
    else()
        set(AMN_PF_RULE_IDENTITY "group { amnvpn }")
    endif()

    configure_file(
        "${CMAKE_SOURCE_DIR}/deploy/data/pf-templates/amn.400.allowPIA.conf.in"
        "${CMAKE_CURRENT_BINARY_DIR}/amn.400.allowPIA.conf"
        @ONLY
    )

    file(COPY_FILE
        "${CMAKE_CURRENT_BINARY_DIR}/amn.400.allowPIA.conf"
        "${CMAKE_SOURCE_DIR}/deploy/data/macos/pf/amn.400.allowPIA.conf"
        ONLY_IF_DIFFERENT
    )
endif()


add_subdirectory(client)

if(NOT IOS AND NOT ANDROID AND NOT MACOS_NE)
    add_subdirectory(service)
endif()

if ((LINUX AND NOT ANDROID) OR (APPLE AND NOT IOS AND NOT MACOS_NE) OR (WIN32))
    include(${CMAKE_SOURCE_DIR}/cmake/CPack.cmake)
endif()
