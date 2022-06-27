using Printf
using QWTWPlot

@printf "pwd = %s \n" pwd()
#ENV["PATH"]=raw"C:\programs\boost\178\lib64-msvc-14.3;"*ENV["PATH"]
lib = "/home/igor/space/qwtw/b1/qwtw/proclib/libqwtw"

qstart(debug = false, qwtw_test = true, libraryName=lib)
qsmw()
x=collect(0.1:0.1:14.0)
y= sin.(x)
qfigure(0, xAxisType=:aLog)
qplot1(x, y, "test", "-eb", 4)

