# -------------------------------------------------
# Project created by QtCreator 2009-11-02T15:00:45
# -------------------------------------------------
TARGET = cw3
TEMPLATE = app
SOURCES += main.cpp \
    individual.cpp \
    tetramino.cpp
HEADERS += individual.h \
    tetrisboard.h \
    tetramino.h \
    game.h \
    engine.h \
    population.h \
    utilities.h
RESOURCES += data.qrc

CONFIG(debug, debug|release) { 
    # Testing
    QT += testlib
    SOURCES += test_board.cpp test_tetramino.cpp
    HEADERS += test_board.h test_tetramino.h
    QMAKE_POST_LINK = ./cw3 \
        t
}

# google-perftools
LIBS += -lprofiler
CONFIG(release):QMAKE_CXXFLAGS += -g

# google-gflags
LIBS += -lgflags

QMAKE_CXXFLAGS += --std=c++0x
