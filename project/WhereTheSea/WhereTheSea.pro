TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    libEasyBMP\
    core\
    gui \


gui.depends = core
core.depends =libEasyBMP

