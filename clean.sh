#!/bin/sh

find . -type f -name '*~' -exec rm '{}' ';'
find . -type f -name '*.orig' -exec rm '{}' ';'

