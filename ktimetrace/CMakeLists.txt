set(KTIMETRACE_SRCS
    acqbuffer.cpp
    adc.cpp
    bufferdialog.cpp
    data.cpp
    dialog.cpp
    ktimetrace.cpp
    ktracecolordialog.cpp
    kttsettings.cpp
    main.cpp
    tracewidget.cpp
    view.cpp
    zoomdialog.cpp
    )

add_executable(ktimetrace ${KTIMETRACE_SRCS})

target_link_libraries(ktimetrace
    Qt5::Widgets
    ZLIB::ZLIB
    Threads::Threads
    )