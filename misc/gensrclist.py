#!/usr/bin/env python

import os

line = ""
with open('sources.lst', 'r') as f:
    lines = f.readlines()

standir = './build/obj'
for line in lines:
    line = line.strip()
    filename = (line.split('/')[-1]).rpartition('.')[0]
    gkd = '%s/%s.o.gkd' % (standir, filename)
    su  = '%s/%s.su' % (standir, filename)
    nm  = '%s/%s.o.nm' % (standir, filename)
    entry = '"%s" = "%s" | "%s" | "%s"' % (line, gkd, su, nm)
    print entry
