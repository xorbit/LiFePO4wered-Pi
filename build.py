#!/usr/bin/env python
from fabricate import *
import os

target = 'lifepo4wered-cli'
sources = ['lifepo4wered-access', 'lifepo4wered-data', 'lifepo4wered-cli']
cflags = '-std=c99 -Wall -O2'.split()
lflags = ''.split()

def build():
    compile()
    link()

def oname(build_dir, filename):
    return os.path.join(build_dir, os.path.basename(filename))

def compile(build_dir='build', flags=None):
    try:
      os.mkdir(build_dir)
    except:
      pass
    for source in sources:
        run('gcc', '-c', source + '.c', '-o', oname(build_dir, source+'.o'), cflags, flags)

def link(build_dir='build', flags=None):
    objects = [oname(build_dir, s + '.o') for s in sources]
    run('gcc', objects, '-o', oname(build_dir, target), lflags, flags)

def check():
    return int(outofdate(build))

def clean():
    autoclean()

def rebuild():
    clean()
    build()

main()
