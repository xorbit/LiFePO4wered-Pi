#!/usr/bin/env python

from __future__ import print_function

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
  },
  'SYSTEMD': {
    'name': 'systemd-check',
    'sources': [
      'systemd-check'
    ],
    'cflags': '-std=c99 -Wall -O2'.split(),
    'lflags': '-lsystemd'.split()
  },
}

def build(target=None):
    compile(target=target)
    link(target=target)

def cli():
    build('CLI')

def so():
    build('SO')

def daemon():
    check_systemd()
    build('DAEMON')

def systemd():
    compile(target="SYSTEMD")
    link(target="SYSTEMD")

def oname(build_dir, target, filename):
    return os.path.join(build_dir, target, os.path.basename(filename))

SYSTEMD = None
def check_systemd():
    global SYSTEMD
    if SYSTEMD is not None:
        return SYSTEMD
    try:
        compile(target="SYSTEMD")
        link(target="SYSTEMD")
    except Exception as exc:
        print("No systemd library found.")
        SYSTEMD=False
        return

    print("Systemd library found.")
    for val in build_targets.values():
        val['cflags'].append('-DSYSTEMD')
        val['lflags'].append('-lsystemd')
    SYSTEMD=True

def compile(build_dir='build', target=None, flags=None):
    if target != None:
      targets = [target]
    else:
      targets = build_targets.keys()
    for target in targets:
      try:
        os.makedirs(oname(build_dir, target, ''))
      except:
        pass
      for source in build_targets[target]['sources']:
          run('gcc', '-c', source + '.c', '-o',
              oname(build_dir, target, source+'.o'),
              build_targets[target]['cflags'], flags)

def link(build_dir='build', target=None, flags=None):
    if target != None:
      targets = [target]
    else:
      targets = build_targets.keys()
    for target in targets:
      objects = [oname(build_dir, target, s + '.o')
                 for s in build_targets[target]['sources']]
      run('gcc', objects, '-o',
          oname(build_dir, target, build_targets[target]['name']),
          build_targets[target]['lflags'], flags)

def check():
    return int(outofdate(build))

def clean():
    autoclean()

def rebuild():
    clean()
    build()

main()
