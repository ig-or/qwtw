import ctypes
from ctypes import *
import os
import numpy as np
import platform

# TODO: make it run in Linux
if platform.system() == 'Linux':
    os.environ["LD_LIBRARY_PATH"] = os.getcwd() + ":" + os.environ["LD_LIBRARY_PATH"]
else:
    os.environ["PATH"] = "c:\\ProgramData\\qwtw;" + os.environ["PATH"]

qq = None
plo = None
plo2 = None

# load the library:
def start():
    global qq, plo, plo2
    lName = 'qwtwc'
    if platform.system() == 'Linux':
        lName = 'libqwtwc.so'

    qq = CDLL(lName)

    qq.get42(0)
    qq.qtstart()

    plo = qq.qwtplot
    plo.argtypes = [POINTER(c_double), POINTER(c_double), c_int, c_char_p, c_char_p, c_int, c_int]
    plo.restype = None

    plo2 = qq.qwtplot2
    plo2.argtypes = [POINTER(c_double), POINTER(c_double), c_int, c_char_p, c_char_p, c_int, c_int, POINTER(c_double)]
    plo2.restype = None

    # print out version info:
    vs = create_string_buffer(450)
    qq.qwtversion(vs, 450)
    versionInfo = vs.raw.decode("utf-8").strip('\x00')
    #print(' version: ' + versionInfo + ' %)\n')

#draw simple plot, just for the testing:
def test():
    global qq, plo
    qq.qwtfigure(0) 
    N = 500
    x = np.linspace(0., 10., N)
    y = np.sin(x)

    xx = x.ctypes.data_as(POINTER(c_double))
    yy = y.ctypes.data_as(POINTER(c_double))

    plo(xx, yy, N, b"bplot 1", b"-m", 2, 2)

    qq.qwttitle(b"the title of a plot")
    qq.qwtxlabel(b"X axis label")
    qq.qwtylabel(b"Y axis label")

# mappings to C library below. For description, please see comments in the original C library qwtwc
def fig(n):
    global qq
    if n == None:
        qq.qwtfigure(0) 
    else:
        qq.qwtfigure(n) 
def map(n):
    global qq
    if n == None:
        qq.topview(0) 
    else:
        qq.topview(n) 

# add horyzontal axis label
def xlab(a):
    global qq
    qq.qwtxlabel(a.encode('utf-8'))

# add vertical axis label    
def ylab(a):
    global qq
    qq.qwtylabel(a.encode('utf-8'))

#add a current plot title    
def name(a):
    global qq
    qq.qwttitle(a.encode('utf-8'))

# remove all the plots    
def clear():
    global qq
    qq.qwtclear()

# set "important" status    
def imp(u):
    global qq
    qq.qwtsetimpstatus(u)       

# draw "main window"    
def mw():
    qq.qwtshowmw()
    
# preprocessing.  Do not use it directly
def pp(x):
    N = len(x)
    if N == 0:
        print('Warning: N == 0')
        return 0, None, None, None

    if type(x) is list:
        x = np.array(x)

    sh = x.shape
    if len(sh) != 2:
        print('wrong first argument (1)')
        raise Exception()
    mp = min(sh)
    if (mp != 2) and (mp != 3):   # FIXME: cannot plot single points and/or lines this way
        print('wrong first argument (2) (need a matrix here)')
        raise Exception()
    mp1 = sh.index(min(sh))
    mp0 = 0
    if mp1 == 0:
        mp0 = 1
        sx = x[0, :]
        sy = x[1, :]
        if mp == 3:
            st = x[2, :]

    else:
        sx = x[:, 0]
        sy = x[:, 1]
        if mp == 3:
            st = x[:, 2]

    nx = sh[mp0]
    xx = sx.flatten().ctypes.data_as(POINTER(c_double))
    yy = sy.flatten().ctypes.data_as(POINTER(c_double))
   
    if mp == 3:
        tt = st.flatten().ctypes.data_as(POINTER(c_double))
    else:
        tt = 0

    return nx, xx, yy, tt

# add a line on a plot:
# x: a list of lists, or preferebly numpy array with the data
# name: name for this line
# style:  a line style
# width: line width
# size: symbols size
#
# style notation:
#    string, 1, 2 or 3 characters;
#
#      last char is always color:
#      'r'  red
#      'd'  darkRed
#      'k'  black
#      'w'  white (quite useless because background is white)
#      'g'  green
#      'G'  darkGreen
#      'm'  magenta
#      'M'  darkMagenta
#      'y'  yellow
#      'Y'  darkYellow
#      'b'  blue
#      'c'  cyan
#      'C'  darkCyan
#
#      first char is always a line style:
#
#      ' ' NoCurve
#      '-' Lines
#      '%' Sticks
#      '#' Steps
#      '.' Dots
#
#       middle char is symbol type (if we need symbols):
#
#       'e' Ellipse
#       'r' Rect
#       'd' Diamond
#       't' Triangle
#       'x' Cross
#       's' Star1
#       'q' Star2
#       'w' XCross
#       'u' UTriangle
#
def line(x, name = "a line", style = "-b", width = 2, size = 1):
    global qq, plo, plo2
    if plo == None:
        raise Exception("qstart() was not called")
    try:
        N, x, y, t,  = pp(x)
        if N == 0: # empty?
            return
        if type(t) is int:  # no time info
            plo(x, y, N, name.encode('utf-8'), style.encode('utf-8'), width, size)
        else:
            plo2(x, y, N, name.encode('utf-8'), style.encode('utf-8'), width, size, t)

    except  Exception as e:
        print('\nERROR: ' + str(e) + '\n\n')

# draw a plot (with many lines) from a list of lists, or big matrix:
def plot(x, name1 = 'just a plot', lines = '', labelX = 'seconds', labelY = ''):
    global qq, plo, plo2
    if lines != '':
        lines = lines.split(', ')
    def co(i): # colors definition
        colors = 'bkmcrgyYG'
        return colors[i % len(colors)]
    def lns(i): # line names
        if lines == '':
            return ('item #' + str(i)).encode('utf-8')
        else:
            i -= 1
            if i < len(lines):
                return lines[i].encode('utf-8')
            else:
                return "????".encode('utf-8')

    if type(x) is list:
        x = np.array(x)

    sh = x.shape
    if len(sh) != 2: # need 2D matrix here
        print('qwtwc: wrong first argument (1)')
        raise Exception()
    mp = min(sh)
    if (mp < 2 ):   # need at least two vectors, X, and Y
        print('wrong first argument (2) (need a matrix here)')
        raise Exception()
    mp1 = sh.index(mp) # columns or rows? lets see....
    mp0 = 0
    qq.qwtfigure(0) 
    if mp1 == 0: # info in raws
        mp0 = 1
        nx = sh[mp0] # real number of 'time points'
        sx = x[0, :]
        xx = sx.flatten().ctypes.data_as(POINTER(c_double))
        for i in range(1, mp):
            sy = x[i, :]
            yy = sy.flatten().ctypes.data_as(POINTER(c_double))
            plo(xx, yy, nx, lns(i), ("-" + co(i)).encode('utf-8'), 2, 3)
    else:
        nx = sh[mp0]
        sx = x[:, 0]
        xx = sx.flatten().ctypes.data_as(POINTER(c_double))
        for i in range(1, mp):
            sy = x[:, i]
            yy = sy.flatten().ctypes.data_as(POINTER(c_double))
            plo(xx, yy, nx, lns(i), ("-" + co(i)).encode('utf-8'), 2, 3)
    name(name1) # a plot name
    xlab(labelX) # x axis label
    ylab(labelY)


   
