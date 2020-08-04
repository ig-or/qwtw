# this will create a file build_info.cpp with some info inside
#    \file build_info.py
#    \author   Igor Sandler
#
#    \version 1.0
#
#

import buildlib
from contextlib import closing
from datetime import datetime
import sys

if len(sys.argv) < 3: 
    print("ERROR in build_info.py; need two arguments:")
    print("	1: source file name (version.txt)")
    print("	2: destination file name (build_info.cc)")
    sys.exit(1)

else: # save project name
    srcName =  sys.argv[1]
    dstName =  sys.argv[2]
	
if len(sys.argv) > 3:
    our_platform = sys.argv[3]
else:
    our_platform = " "
    
#read version info:
__version__ = buildlib.get_StringFrom(srcName)

#time: 
__currentTime__ = str(datetime.now())

#put everything to a file:
try:
    with closing(open(dstName,'wt')) as bif:
        print('writing build info into ' + dstName)
        buildlib.writeCString(bif, 'VERSION', __version__)
        buildlib.writeCString(bif, 'OUR_PLATFORM', our_platform)
		#writeString(bif, 'COMPILE_TIME', __currentTime__)
except:
    print('ERROR in build_info.py; can not create output file ' + dstName)
    raise
	
