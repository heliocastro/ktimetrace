# This file is part of ktimetrace.

# ktimetrace is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# ktimetrace is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with ktimetrace.  If not, see <https://www.gnu.org/licenses/>.

# (C) 2018 by Helio Chissini de Castro <helio@kde.org>

set(KTIMETRACE_SRCS
    main.cpp
    acqbuffer.cpp
    adc.cpp
    bufferdialog.cpp
    data.cpp
    dialog.cpp
    ktimetrace.cpp
    ktracecolordialog.cpp
    kttsettings.cpp
    tracewidget.cpp
    view.cpp
    zoomdialog.cpp
    )

#qt5_add_resources(UI_RESOURCES ktimetrace.qrc)

qt2_wrap_moc(KTIMETRACE_SRCS
    SOURCES
        bufferdialog.h
        ktimetrace.h
        kttsettings.h
        ktracecolordialog.h
        tracewidget.h
        data.h
        dialog.h
        view.h
        zoomdialog.h
    )

add_executable(ktimetrace
    ${KTIMETRACE_SRCS}
    ${UI_RESOURCES}
    )

target_compile_definitions(ktimetrace PUBLIC
    PACKAGE="ktimetrace"
    VERSION="${PROJECT_VERSION}"
    )

target_link_libraries(ktimetrace
    kde2::kdecore
    kde2::kdeui
    ZLIB::ZLIB
    Threads::Threads
    Comedi::Comedi
    )
