#***
#   configure or build or clean  one single  project
#***
#

import sys
import os
import shutil
from subprocess import call
from datetime import datetime
from contextlib import closing
import buildlib
from optparse import OptionParser

usage = "usage: %prog [options]"
parser = OptionParser(usage=usage, prog="xmake")
parser.add_option('--proj', dest='proj', action='store', default='', help="our project (relative ?) folder")
parser.add_option('--op', dest='op', action='store', default='', help="operation: 'c' (configure) or 'b' (build) or 'd' (delete)")
parser.add_option('--pf', dest='pf', action='store', default='', help="our platform ('vs2015-x64' ... 'jom' ... 'linux-x64' .. 'all' etc)")
parser.add_option('--bt', dest='bt', action='store', default='', help="build type: 'release' or 'debug' (for jom or unix)")

opt, args = parser.parse_args()
if not opt.proj:
    print('error: you have to provide an info about project location')
    quit()
    
proj = opt.proj
if not opt.pf:
    pf = '?'
else:
    pf = opt.pf
buildType = 'release'
if opt.bt:
    buildType = opt.bt
    
operation = 'c'
if opt.op:
    operation = opt.op

#  ---------------------------------------------
basePath = os.getcwd() # remember this path
try:
    if operation == 'd':
        buildlib.cleanProject(proj, pf, buildType)
        
    if operation == 'c':
        buildlib.config(proj, pf, buildType)
        
    if operation == 'b':
        buildlib.smallBuild(proj, pf, buildType)

except  Exception as einst: # 
    print("operation faild: " + str(einst))
    quit() 

#print 'moving back into folder '    + basePath + ' .... '
os.chdir(basePath) 
#print 'done.'



