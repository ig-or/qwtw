# A set of useful functions for python
#    \file buildlib.py
#    \author   Igor Sandler
#
#    \version 1.0
#
#


import sys
import os
from os.path import exists, join, abspath
from os import pathsep
import errno
import glob
import shutil
import subprocess
from subprocess import call
import string
from datetime import datetime
from contextlib import closing
from contextlib import closing

# move to folder 'fName' and may be create it:
def setFolder(fName):
    try:
        fName = fName.replace('/', pathsep)
        fName = fName.replace('\\', pathsep)
        a = fName.split(pathsep)
        for  x in a:
            if not os.path.exists(x):
                os.mkdir(x)
            os.chdir(x)
    except Exception as e: 
        print(e)
        raise  Exception('ERROR: setFolder faild... fName=' + fName)

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST:
            pass
        else: raise
    return path

def copybins(src_path, src_list, dst_path):
    #count = 0
    mkdir_p(dst_path)
    for exp in src_list:
        count = 0
        for prog in glob.glob(os.path.join(src_path, exp)):
            dfName = os.path.join(dst_path, os.path.basename(prog))
            try:
                if (os.path.exists(dfName)):
                    os.remove(dfName)
            except:
                print("buildlib.py: copybins error; file = " + str(fdname))
                raise

            shutil.copy2(prog, dst_path)
            count = count + 1
            print('copying  ' + prog + '   into    ' + dst_path	)
        if (count == 0):
            msg = 'buildlib::copybins faild; src= ' + src_path + ' ' + str(src_list) + '; destination= ' + dst_path
            raise Exception(msg)

# from http://code.activestate.com/recipes/52224-find-a-file-given-a-search-path/
def search_file(filename, search_path):
    """Given a search path, find file
    """
    file_found = 0
    paths = string.split(search_path, pathsep)
    for path in paths:
        if exists(join(path, filename)):
            file_found = 1
            break
    if file_found:
        return abspath(join(path, filename))
    else:
        return None

def get_StringFrom(versionFile):
    try:
        with closing(open(versionFile,'rt')) as vf:
            __version__ = vf.read(64)
        if __version__[-1:] == '\n':
            __version__ = __version__[:-1]
        return __version__
    except:
        raise Exception('ERROR reading version information; can not open ' + versionFile + '\n')
        return None #		
        
# read version info:	  
def getVersionString():
    try:
        fileName = search_file('version.txt', '.;../;../../;../../../;../../../../;../../../../../')
        if not fileName:
            raise Exception('ERROR: getVersionString() can not find version.txt file')
            return None #
        __version__ = get_StringFrom(fileName)
        return __version__
    except:
        raise Exception('ERROR reading version information; can not open version.txt')
        return None #
        #raise
		
def writeCString(file, macro, value):
    file.write('extern const char ' + macro + '[] = \"' + value + '\"; \n')
		

def getBranchName():
    branch = 'no_branch' #  for the current project
    ok = 0
    git_output = ""
    try:
        p = subprocess.Popen('git branch', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in p.stdout.readlines():
            #print '    getBranchName(): ' + line		
            line = line.decode('utf-8')
            git_output = git_output + line + "; "
            if line[0] == '*':
                ok = 1
                branch = line[2:-1]
                #print '    getBranchName(): OK!'
                break
        rv = p.wait()
    except Exception as einst: # 
        print(einst)
        raise Exception('getBranchName(): git didnt work 1' + '[' + str(einst.args) + ']')
    if (ok == 0):
        raise Exception('getBranchName(): error while parsing git output: ' + git_output)
        
    dirty = ''
    try:
        p = subprocess.Popen('git diff --quiet || echo dirty', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        dirty = p.stdout.readline()
        dirty = dirty.decode('utf-8')
        if len(dirty) > 0:
            branch = branch + ' (' + dirty[0:-2] + ')'
        rv = p.wait()
    except Exception as einst: # 
        raise Exception('getGitSHA(): git didnt work 2' + '[' + einst.args + ']')

        
    di = ''
    try:
        p = subprocess.Popen('git diff-index --quiet  HEAD -- || echo dirty_index', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        di = p.stdout.readline()
        di = di.decode('utf-8')
        if len(di) > 0:
            branch = branch + ' (' + di[0:-2] + ')'
        rv = p.wait()
    except Exception as einst: # 
        raise Exception('getGitSHA(): git didnt work 3' + '[' + einst.args + ']')

    
    return branch
    
  
    
def getGitSHA():
    sha = ' '
    ok = 0
    try:
        p = subprocess.Popen('git rev-parse @', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        sha = p.stdout.readline().decode('utf-8')
        rv = p.wait()
    except Exception as einst: # 
        raise Exception('getGitSHA(): git didnt work' + '[' + einst.args + ']')
    return sha[0:-1]


def getGitInfo():
    info = ' '
    op = os.getcwd()
    p1, p2 = os.path.split(op)
    #print 'p1 = ' + p1 + ';  p2 = ' + p2

    #print 'getGitInfo: p2 = ' + p2
    try:
        info = '[' + p2 + ']: ' + ' [' + getGitSHA() + ']'
    except  Exception as einst: # 
        raise Exception('getGitInfo(): git didnt work' + '[' + einst.args + ']')
    return info
        
    
def getGitInfoExt(projectDir, xqDir, extDir):
    projInfo = '' #  for the current project
    xqInfo = '' # for the xqtools
    extInfo = '' # for the externals repo
    op = os.getcwd()
    try:
        if len(projectDir) > 1:
            os.chdir(projectDir)
            projInfo = getGitInfo()
            
        if len(xqDir) > 1:
            os.chdir(xqDir)
            xqInfo = getGitInfo()
            
        if len(extDir) > 1:
            os.chdir(extDir)
            extInfo = getGitInfo()
            
        os.chdir(op)
        
    except  Exception as einst: # 
        print('getGitInfoExt exception: '  + '[' + str(einst.args) + ']' )
        raise Exception('getGitInfoExt(): git didnt work' + '[' + str(einst.args) + ']')    
    
    return projInfo, xqInfo, extInfo
    

#CUR_DIR = os.path.normpath(os.path.abspath(os.getcwd()))	
#print os.path.join(CUR_DIR, '..\\', 'obj', getBranchName(), 'release', 'geoclean_enterprise')	

# config or build the project
#    task == 'c' => cmake config;   task == 'b' =>jom build
def buildProject(projectPath, task='b', pf='w', printLines='n'):
    basePath = os.getcwd() # remember this path
    if os.path.exists(projectPath + "/CMakeLists.txt"):  # we find project file!
        print ( "processing "  + projectPath + ':'   )
    else:
        print ( "ERROR: can not find project " + projectPath  )
        sys.exit(2)
        
    # build type for cmake, and build directory name:
    if pf == 'w':
        makeCmd = 'jom -j 9 all'
        generators = ['"NMake Makefiles JOM"', '"NMake Makefiles JOM"']
        cmakeFlags = ['-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_BUILD_TYPE=Release']
        buildDirs = ['debug', 'release']
    if pf == 'u':
        makeCmd = 'make -j 9'
        generators = ['"Unix Makefiles"', '"Unix Makefiles"']
        cmakeFlags = ['-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_BUILD_TYPE=Release']
        buildDirs = ['debug', 'release']

    for ii in [0, 1]:
        os.chdir(basePath) 
        buildPath = os.path.join(projectPath, 'build', buildDirs[ii])
        setFolder(buildPath)
        
        if task == 'c':  # run cmake
            cmakeCmd = 'cmake -G ' + generators[ii] + ' ' +  cmakeFlags[ii] + ' ../../.'
            ok = 0
            errorLine = ''
            rv = 0
            try:
                #print('running ' + cmakeCmd + ' in ' + buildPath)
                p = subprocess.Popen(cmakeCmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                for line in p.stdout.readlines():
                    if printLines == 'y':
                        print (line    )
                    if line.lower().find('error') >= 0:
                        print (line)
                        errorLine = line
                    #else:
                        #print line
                    if line.lower().find('errors occurred') >= 0:
                        ok = 1  # not OK
                        print (line)
                        errorLine = line
                rv = p.wait()
            except  Exception as einst: # 
                raise Exception('cmake faild (' + cmakeCmd + ')')
            if ok == 1:
                raise Exception('cmake faild: (' + cmakeCmd + ')' + errorLine)
            if rv != 0:
                raise Exception('cmake (' + cmakeCmd + ') faild: return code = ' + str(rv))
                
        if task == 'b': #ok , let's jom it:
            if ii == 0:
                continue

            ok = 0
            errorLine = ''
            try:
                p = subprocess.Popen(makeCmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                for line in p.stdout.readlines():
                    #print(line)
                    if line.find('error') >= 0:
                        print(line)
                        errorLine = line
                        ok = 1
                rv = p.wait()
            except  Exception as einst: # 
                raise Exception('make faild (' + makeCmd + ') for projectPath ' + buildDirs[ii])
            if ok == 1:
                raise Exception('make faild: ' + errorLine)
            if rv != 0:
                raise Exception('make ' +  projectPath + ' (' + buildDirs[ii] + ') faild: return code = ' + str(rv))

def cmdLogOutput(logList):
    if len(logList) == 0:
        return
    print('---------------------------------------------------' )   
    for line1 in logList:
        print(line1)
    print('---------------------------------------------------'  ) 
                
def getBuildPath(projectPath, platform = 'jom', type = 'release'):
    if os.path.exists(projectPath + "/CMakeLists.txt"):  # we find project file!
        print("processing "  + projectPath + ': ')
    else:
        raise Exception("ERROR: can not find project " + projectPath)    
        
    buildFolder = platform
    if platform == 'jom':
        buildFolder += '-' + type

    return os.path.join(projectPath, 'build', buildFolder)
    
def cleanProject(projectPath, platform = 'all', type = 'release'):
    if platform == 'all':
        pp = projectPath + '/build'
    else:
        pp = getBuildPath(projectPath, platform, type)
    if not os.path.exists(pp):
        print('folder ' + pp + ' does not exist. ')
        return
        
    print('deleting folder ' + pp)
    shutil.rmtree(pp)        
    print('done.')

# build project, configured previously              
def smallBuild(projectPath, platform = 'jom', type = 'release'):
    setFolder(getBuildPath(projectPath, platform, type))
    
    cmd = ''
    if platform == 'jom':
        cmd = 'jom'
    else:
        if platform == 'linux-x64':
            cmd = 'make'
        else:
            es =  'error platform ' + platform + ' not supported yet '
            raise Exception(es)
        
    ostr = []
    try:
        print('running ' + cmd)
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in p.stdout.readlines():
            ostr.append(line)
            print(line			)
        rv = p.wait()
    except  Exception as einst: # 
        cmdLogOutput(ostr)
        raise Exception('build with ' + cmd + ' failed ')       
    print( 'return code: ' + str(rv))
    if rv != 0:
        print('looks like build failed; output below: ')
        cmdLogOutput(ostr)
    else:    
        print('done.'    )
    
            
def config(projectPath, platform = '?', type = 'release'):
    cmakeCmd = 'cmake '
    if 'VCPKG_ROOT' in os.environ:
        cmakeCmd += ' -DCMAKE_TOOLCHAIN_FILE=' + os.environ['VCPKG_ROOT'] + '/scripts/buildsystems/vcpkg.cmake '
    if 'VSPLATFORM' in os.environ:
        if platform == '?':
            platform = os.environ['VSPLATFORM']
            print('using ' + platform + ' as target platform')
        else:
            if platform != os.environ['VSPLATFORM']:
                print('WARNING: using target platform = ' + platform)
    else:
        if platform == '?':
            print('ERROR: no platform selected')
            return
    setFolder(getBuildPath(projectPath, platform, type))
    cmakeGkey = {
        'linux-x32': '"Unix Makefiles"',
        'linux-x64': '"Unix Makefiles"',
        'jom': '"NMake Makefiles JOM"',
        'vs2010-x32': '"Visual Studio 10 2010"',
        'vs2010-x64': '"Visual Studio 10 2010 Win64"',
        'vs2013-x32': '"Visual Studio 12 2013"',
        'vs2013-x64': '"Visual Studio 12 2013 Win64"',
        'vs2015-x32': '"Visual Studio 14 2015"',
        'vs2015-x64': '"Visual Studio 14 2015 Win64"',
        'vs2017-x32': '"Visual Studio 15 2017"',
        'vs2017-x64': '"Visual Studio 15 2017 Win64"',
        'vs2019-x64': '"Visual Studio 16 2019"'  #' -A Win64' 
    }
    try:
        cmakeCmd += ' -G ' + cmakeGkey[platform] 
        if not 'vs' in platform:
            if type == 'release':
                cmakeCmd += ' -DCMAKE_BUILD_TYPE=Release '
            else:    
                cmakeCmd += ' -DCMAKE_BUILD_TYPE=Debug '
        else:
            print('creating Visual Studio project (platform=' + platform + ') ')
        cmakeCmd += ' ../../.'
    except  Exception as einst:
        print('cannot set up a platform: ' + str(einst))
        raise Exception('cmake faild: (' + cmakeCmd + ')')
    
    ok = 0
    rv = 0
    cmakeOutput = []
    try:
        print('running ' + cmakeCmd)
        p = subprocess.Popen(cmakeCmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in p.stdout.readlines():
            #cmakeOutput.append(line)
            print(line.decode('utf-8'))
            #if line.lower().find('error') >= 0:
             #   print line
            #else:
                #print line
            #if line.lower().find('errors occurred') >= 0:
            #    ok = 1  # not OK
            #if ok != 0:
            #    print line

        rv = p.wait()
    except  Exception as einst: # 
        #cmdLogOutput(cmakeOutput)
        raise Exception('cmake faild #1 (' + cmakeCmd + ')')
        
    if ok == 1:
        #cmdLogOutput(cmakeOutput)
        raise Exception('cmake faild: (' + cmakeCmd + ')')
    if rv != 0:
        #cmdLogOutput(cmakeOutput)
        raise Exception('cmake (' + cmakeCmd + ') faild: return code = ' + str(rv))

    
	
