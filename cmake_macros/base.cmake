# using ${TARGET_LIB_PATH} to find_package
macro(FIND_PKG_PATH pkg_name alias)
    string(TOLOWER "${alias}" alias_lower)
    string(TOUPPER "${alias}" alias_upper)

    set(TARGET_${alias_upper}_PATH "" CACHE PATH "${pkg_name} install path")
    if(EXISTS ${TARGET_${alias_upper}_PATH})
    	message(STATUS "Directory to search ${pkg_name} at ${TARGET_${alias_upper}_PATH}")
    	list(APPEND CMAKE_PREFIX_PATH ${TARGET_${alias_upper}_PATH})
    else()
    	message(STATUS "File/Directory at variable TARGET_${alias_upper}_PATH not exists! ${TARGET_${alias_upper}_PATH}")
    endif()
    find_package(${pkg_name} REQUIRED)
endmacro(FIND_PKG_PATH)