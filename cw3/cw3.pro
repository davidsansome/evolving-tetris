# -------------------------------------------------
# Project created by QtCreator 2009-11-02T15:00:45
# -------------------------------------------------
TARGET = cw3
TEMPLATE = app
SOURCES += main.cpp \
    individual.cpp \
    tetramino.cpp \
    blockselector_random.cpp \
    blockselector_sequence.cpp \
    individualbase.cpp \
    gamemapper.cpp \
    utilities.cpp \
    game.cpp
HEADERS += individual.h \
    tetrisboard.h \
    tetramino.h \
    game.h \
    engine.h \
    population.h \
    utilities.h \
    blockselector_random.h \
    blockselector_sequence.h \
    individualbase.h \
    gamemapper.h
PROTOBUF_SOURCES += messages.proto

CONFIG(release):DEFINES += NDEBUG # For cassert

CONFIG(debug, debug|release) { 
    # Testing
    QT += testlib
    SOURCES += test_board.cpp \
        test_tetramino.cpp \
        test_generators.cpp
    HEADERS += test_board.h \
        test_tetramino.h \
        test_generators.h
    QMAKE_POST_LINK = ./cw3 \
        t
}

# DEFINES += NO_QT_STUFF

# google-perftools
# LIBS += -lprofiler
# CONFIG(release):QMAKE_CXXFLAGS += -g

# google-gflags
LIBS += -lgflags
QMAKE_CXXFLAGS += --std=c++0x

# protobuf
protobuf_cpp.input = PROTOBUF_SOURCES
protobuf_cpp.output = ${QMAKE_FILE_IN_BASE}.pb.h
protobuf_cpp.commands = protoc \
    --cpp_out=. \
    ${QMAKE_FILE_IN}
protobuf_cpp.variable_out = SOURCES
OTHER_FILES += $${PROTOBUF_SOURCES}
QMAKE_EXTRA_COMPILERS += protobuf_cpp
LIBS += -lprotobuf
