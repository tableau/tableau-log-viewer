#!/bin/bash
add-apt-repository -y ppa:beineri/opt-qt57-trusty
apt-get update -y
apt-get install -y \
  qt57declarative \
  qt57graphicaleffects \
  qt57quickcontrols \
  qt57svg \
  qt57tools \
  qt57webengine
