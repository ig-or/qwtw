#    increase a build number and create a cpp name with this info.
#    \file incbuildnumber.py
#    \author   Igor Sandler
#    \version 1.0
#
#


import buildlib
from contextlib import closing
import sys
from datetime import datetime
import string
import traceback

print('========creating build number=========')
useExtFormat = False
projDir = ''; xqDir = ''; extDir = ''

if len(sys.argv) < 3: 
    print("ERROR in incbuildnumber.py; need two arguments:")
    print("	1:  file name ")
    print("	2:  output file name ")
    sys.exit(1)

fName =  sys.argv[1]  # file with a build number
dstName = sys.argv[2] # destination file
	
if len(sys.argv) > 3:  # use extended info format:
    useExtFormat = True
    projDir = sys.argv[3]    # project folder

if len(sys.argv) > 4:    # xqtools repo folder
    xqDir = sys.argv[4]
    
if len(sys.argv) > 5:     # extlib repo folder
    extDir = sys.argv[5]

try:
    x = buildlib.get_StringFrom(fName)
    x = int(x)
except:
    x = 0
    print("WARNING: cannot read a build number from a file {" + fName + "}")
x = x + 1

#time: 
__currentTime__ = str(datetime.now())

#put everything back to a file:
try:
    with closing(open(fName,'wt')) as bif1:
        bif1.write(str(x))
except:
    print('ERROR in incbuildnumber.py; can not create output file ' + fName)
    raise
	
#put everything to a file:
try:
    try:
        os.remove(dstName)
    except:
        pass

    with closing(open(dstName,'wt')) as bif:
        print('BN #' + str(x))
        print('build time = ' + __currentTime__)
        buildlib.writeCString(bif, 'BUILD_NUMBER', str(x))
        buildlib.writeCString(bif, 'COMPILE_TIME', __currentTime__)
        
        # update GIT related info:
        gitInfo = ''
        if useExtFormat:
            pInfo = ' ';  xqInfo = ' ';   eInfo = ' '
            try:
                #pInfo, xqInfo, eInfo = buildlib.getGitInfoExt(projDir, xqDir, extDir)
                pInfo, xqInfo, eInfo = buildlib.getGitInfoExt(projDir, xqDir, extDir)
            except Exception as einst: #:
                print('WARNING: cannot get GIT related info [' + str(einst.args) + ']'	)
                traceback.print_exc()
            #gitInfo = pInfo + ' \n' + xqInfo + ' \n' + eInfo
            gitInfo = pInfo# + ' \\n'# + xqInfo + ' \\n' + eInfo

        else:
            branch = ' '	
            try:
                branch = buildlib.getBranchName()
            except Exception as einst: #:
                print('WARNING: cannot get branch name [' + str(einst.args) + ']'	)
            gitInfo = buildlib.getGitSHA()
            
        print('GIT info: ' + gitInfo)    
        buildlib.writeCString(bif, 'GIT_INFO', gitInfo)
		
except:
    print('ERROR: can not create output file ' + dstName)
    raise

	
	
	
	
