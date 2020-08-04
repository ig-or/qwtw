
import sys
import os
from subprocess import call
from datetime import datetime
from contextlib import closing
import buildlib

projects = ['dss', 'qwtw/c_lib', 'qwtw/cpp_lib', 'qwtw/qwtwtest', 
        'ips2xq', 'xq2ips', 'nav/test/pptest', 
        'xqloader', 'rdm_reframer/source', 
        'rdmrealtime/c_lib', 'rdmrealtime/c_lib_test', 'rdmrealtime/dss_test']

os.chdir('../')  #  go to one level up
basePath = os.getcwd() # remember this path
for proj in projects:
    buildlib.cleanProject(proj)
    os.chdir(basePath) 


