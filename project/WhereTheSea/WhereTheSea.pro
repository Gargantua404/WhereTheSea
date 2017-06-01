TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    libEasyBMP\
    libGeographic\
    core\
    gui


gui.depends = core
gui.depends = libGeographic
gui.depends = libEasyBMP
