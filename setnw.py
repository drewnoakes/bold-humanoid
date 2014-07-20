#!/usr/bin/env python

import sys
import ConfigParser

if len(sys.argv) < 2:
    print 'Give ESSID'
    exit(1)

config = ConfigParser.RawConfigParser()

config.read(r'/etc/wicd/wireless-settings.conf')

sectionName = 'essid:' + sys.argv[1]
print config.sections()

print 'Section name: ' + sectionName

if not sectionName in config.sections():
    print 'ESSID not in configuration'
    exit

# Turn off automatic connection to all networs
for section in config.sections():
    config.set(section, 'automatic', '0')

# Turn on automatic on selected ESSID
config.set(sectionName, 'automatic', '1')

with open(r'/etc/wicd/wireless-settings.conf', 'wb') as configfile:
    config.write(configfile)
