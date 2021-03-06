#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 1999-2011 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the
#  Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#=============================================================================

include_directories(
   .
   ${CMAKE_CURRENT_BINARY}
   ${PROJECT_SOURCE_DIR}
   ${PROJECT_SOURCE_DIR}/muse
   ${PROJECT_SOURCE_DIR}/muse/widgets
   ${PROJECT_SOURCE_DIR}/synti
   # Make sure to include the uic generated headers paths.
   ${CMAKE_CURRENT_BINARY_DIR}/libsimpleplugin
   )

set (SubDirs
      libsimpleplugin
      libsynti
      deicsonze
      # deicsonze2
      organ
      # simpledrums
      simpledrums2
      vam
      )

if (ENABLE_EXPERIMENTAL)
      set (SubDirs ${SubDirs} s1 )
endif (ENABLE_EXPERIMENTAL)

if (HAVE_FLUIDSYNTH)
      set (SubDirs ${SubDirs}  fluidsynth ) # removed fluid, fluidsynth should supercede it in every way.
endif (HAVE_FLUIDSYNTH)

subdirs(${SubDirs})


