#!/usr/bin/env python
from fabricate import *
import os

build_targets = {
  'CLI': {
    'name': 'lifepo4wered-cli',
    'sources': [
      'lifepo4wered-access',
      'lifepo4wered-data',
      'lifepo4wered-cli'
    ],
    'cflags': '-std=c99 -Wall -O2'.split(),
    'lflags': ''.split()
  },
  'DAEMON': {
    'name': 'lifepo4wered-daemon',
    'sources': [
      'lifepo4wered-access',
      'lifepo4wered-data',
      'lifepo4wered-daemon'
    ],
    'cflags': '-std=c99 -Wall -O2'.split(),
    'lflags': ''.split()
  },
  'SO': {
    'name': 'liblifepo4wered.so',
    'sources': [
      'lifepo4wered-access',
      'lifepo4wered-data'
    ],
    'cflags': '-std=c99 -Wall -O2 -fpic'.split(),
    'lflags': '-shared'.split()
  }
}
    
def build(target=None):
    compile(target)
    link(target)

def cli():
    build('CLI')

def so():
    build('SO')

def daemon():
    build('DAEMON')

def oname(build_dir, filename):
    return os.path.join(build_dir, os.path.basename(filename))

def compile(build_dir='build', target=None, flags=None):
    if target != None:
      targets = [target]
    else:
      targets = build_targets.keys()
    for target in targets:
      try:
        os.mkdir(target)
      except:
        pass
      for source in build_targets[target]['sources']:
          run('gcc', '-c', source + '.c', '-o', oname(target, source+'.o'), build_targets[target]['cflags'], flags)

def link(build_dir='build', target=None, flags=None):
    if target != None:
      targets = [target]
    else:
      targets = build_targets.keys()
    for target in targets:
      objects = [oname(target, s + '.o') for s in build_targets[target]['sources']]
      run('gcc', objects, '-o', oname(target, build_targets[target]['name']), build_targets[target]['lflags'], flags)

def check():
    return int(outofdate(build))

def clean():
    autoclean()

def rebuild():
    clean()
    build()

main()
