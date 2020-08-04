# this script will output version number string
# taken from a text file 'version.txt'
#    \file get_version.py
#    \author   Igor Sandler
#    \version 1.0
#
#
#


import buildlib 

__version__ = buildlib.getVersionString()
if (__version__ == None):
	print 'no_version'  # 'ERROR reading version information; can not open version.txt'
	raise Exception
else:
	print __version__