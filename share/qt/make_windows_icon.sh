#!/bin/bash
# create multiresolution windows icon
ICON_DST=../../src/qt/res/icons/SoleCoin.ico

convert ../../src/qt/res/icons/SoleCoin-16.png ../../src/qt/res/icons/SoleCoin-32.png ../../src/qt/res/icons/SoleCoin-48.png ${ICON_DST}
