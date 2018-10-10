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

find_package(PkgConfig REQUIRED)
pkg_search_module(COMEDI REQUIRED comedilib)

if(COMEDI_FOUND)
    if(NOT TARGET Comedi::Comedi)
        add_library(Comedi::Comedi UNKNOWN IMPORTED)
        set_target_properties(Comedi::Comedi PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${COMEDI_INCLUDE_DIRS}")

        if(COMEDI_LIBRARIES)
            set_property(TARGET Comedi::Comedi APPEND PROPERTY
                IMPORTED_LOCATION "-l${COMEDI_LIBRARIES}")
        endif()
    endif()
    message(STATUS "Found Comedi ${COMEDI_VERSION}...")
endif()