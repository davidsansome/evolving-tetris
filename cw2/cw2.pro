TARGET = cw2
TEMPLATE = app
SOURCES += main.cpp \
    individual.cpp \
    tetramino.cpp \
    engine.cpp \
    population.cpp
CUSOURCES += boardrating.cu
HEADERS += individual.h \
    tetrisboard.h \
    tetramino.h \
    game.h \
    engine.h \
    population.h \
    boardrating.h
FORMS += 
RESOURCES += data.qrc
CONFIG(debug, debug|release) { 
    # Testing
    QT += testlib
    SOURCES += test_board.cpp
    HEADERS += test_board.h
    #QMAKE_POST_LINK = ./cw2 \
    #    t
}

# google-perftools
LIBS += -lprofiler
CONFIG(release):QMAKE_CXXFLAGS += -g

# cuda
LIBS += -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart
INCLUDEPATH += /usr/local/cuda/include
cu.name = Cuda \
    ${QMAKE_FILE_IN}
cu.input = CUSOURCES
cu.CONFIG += no_link
cu.variable_out = OBJECTS
cu.output = ${QMAKE_FILE_BASE}.cu.o
cu.commands = nvcc \
    -o \
    ${QMAKE_FILE_BASE}.cu.o \
    -c \
    ${QMAKE_FILE_NAME} \
    --compiler-bindir=/home/david/NVIDIA_GPU_Computing_SDK/bin
QMAKE_EXTRA_COMPILERS += cu
OTHER_FILES += $${CUSOURCES}
