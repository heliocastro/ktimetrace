# This file is part of ktimetracer.

# ktimetracer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# ktimetracer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with ktimetracer.  If not, see <https://www.gnu.org/licenses/>.

# (C) 2018 by Helio Chissini de Castro <helio@kde.org>

#.rst:
# Findcomedi
# --------
#
# Find the comedi includes and library.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``Comedi::Comedi``, if
# Comedi has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   COMEDI_INCLUDE_DIRS   - where to find comedi.h, etc.
#   COMEDI_LIBRARIES      - List of libraries when using comedi.
#   COMEDI_FOUND          - True if comedi found.
#
# ::
#

find_path(COMEDI_INCLUDE_DIR comedi.h PATH_SUFFIXES include)

if(NOT COMEDI_LIBRARIES)
    find_library(COMEDI_LIBRARY_RELEASE NAMES comedi PATH_SUFFIXES lib lib64)
    find_library(COMEDI_LIBRARY_DEBUG NAMES comedid PATH_SUFFIXES lib lib64)

    include(SelectLibraryConfigurations)
    select_library_configurations(COMEDI)
else()
    file(TO_CMAKE_PATH "${COMEDI_LIBRARIES}" COMEDI_LIBRARIES)
endif()

set(COMEDI_VERSION_STRING "0.8.1")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Comedi
                              REQUIRED_VARS COMEDI_LIBRARIES COMEDI_INCLUDE_DIR
                              VERSION_VAR COMEDI_VERSION_STRING)

if (COMEDI_FOUND)
    include(CheckSymbolExists)
    include(CMakePushCheckState)
    cmake_push_check_state()
    set(CMAKE_REQUIRED_QUIET ${comedi_FIND_QUIETLY})
    set(CMAKE_REQUIRED_INCLUDES ${COMEDI_INCLUDE_DIR})
    set(CMAKE_REQUIRED_LIBRARIES ${COMEDI_LIBRARIES})
    check_symbol_exists(comedi_perror "comedilib.h" COMEDI_NEED_PREFIX)
    cmake_pop_check_state()

    if(NOT TARGET Comedi::Comedi)
        add_library(Comedi::Comedi UNKNOWN IMPORTED)
        set_target_properties(Comedi::Comedi PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${COMEDI_INCLUDE_DIRS}")

        if(COMEDI_LIBRARY_RELEASE)
            set_property(TARGET Comedi::Comedi APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(Comedi::Comedi PROPERTIES
                IMPORTED_LOCATION_RELEASE "${COMEDI_LIBRARY_RELEASE}")
        endif()

        if(COMEDI_LIBRARY_DEBUG)
            set_property(TARGET Comedi::Comedi APPEND PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(Comedi::Comedi PROPERTIES
                IMPORTED_LOCATION_DEBUG "${COMEDI_LIBRARY_DEBUG}")
        endif()

        if(NOT COMEDI_LIBRARY_RELEASE AND NOT COMEDI_LIBRARY_DEBUG)
            set_property(TARGET Comedi::Comedi APPEND PROPERTY
                IMPORTED_LOCATION "${COMEDI_LIBRARY}")
        endif()
    endif()
endif()

mark_as_advanced(COMEDI_INCLUDE_DIR)