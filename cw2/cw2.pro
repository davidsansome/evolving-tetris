# -------------------------------------------------
# Project created by QtCreator 2009-11-02T15:00:45
# -------------------------------------------------
TARGET = cw2
TEMPLATE = app
SOURCES += main.cpp \
    individual.cpp \
    tetramino.cpp \
    engine.cpp \
    population.cpp
HEADERS += individual.h \
    tetrisboard.h \
    tetramino.h \
    game.h \
    engine.h \
    population.h
FORMS += 
RESOURCES += data.qrc
CONFIG(debug, debug|release) { 
    # Testing
    QT += testlib
    SOURCES += test_board.cpp
    HEADERS += test_board.h
    QMAKE_POST_LINK = ./cw2 \
        t
}

# google-perftools
#LIBS += -lprofiler
#CONFIG(release):QMAKE_CXXFLAGS += -g
