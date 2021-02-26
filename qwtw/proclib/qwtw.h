/**
QWT-based 2D plotting library.
*/


#pragma once

#ifdef WIN32
    #ifdef qwtwcEXPORTS	
        #define qwtwc_API __declspec(dllexport) // Q_DECL_EXPORT 
    #else
        #define qwtwc_API  __declspec(dllimport) // Q_DECL_IMPORT
    #endif
#else
    #define qwtwc_API
#endif


#ifdef __cplusplus
	extern "C" {
#endif

/**  this is just for testing
 *  \return 42
*/
qwtwc_API 	int get42(int n);

/** starting  QT proc, if not started yet.
 * 	return 0 if all is OK, 
 *  not 0 means nothing is working.
 * 
 * \param mdp path to the marble data files; If "0" than not used
 * \param mpp path to the marble plugin files; If "0" than not used
 **/ 
#ifdef USEMARBLE
qwtwc_API	int qtstart(const char* mdp, const char* mpp);

/** starting  QT proc, if not started yet.
 * 	return 0 if all is OK, 
 *  not 0 means nothing is working.
 * 
 * \param mdp path to the marble data files; If "0" than not used
 * \param mpp path to the marble plugin files; If "0" than not used
 * \param level debug level from 0 to 10; 10 is the biggest.
 **/ 
qwtwc_API	int qtstart_debug(const char* mdp, const char* mpp, int level);
#else
qwtwc_API	int qtstart();
qwtwc_API	int qtstart_debug(int level);
#endif

/**  print version info string (with 'snprintf')
	@param[out] vstr string with version info
    \param vstr_size size of vstr
	@return number of bytes in vstr
*/
qwtwc_API		int qwtversion(char* vstr, int vstr_size);

/**  create (and draw) new plot with ID 'n';  make this plot 'active'
     if plot with this ID already exists, it will be made 'active'

     if n == 0 then will add another plot anyway
    @param[in] n this plot ID
*/
qwtwc_API 	void qwtfigure(int n);


#ifdef USEMARBLE
/**  create (and draw) new map plot with ID 'n'.
@param[in] n this plot ID
*/
qwtwc_API 	void qwtmap(int n);
#endif
#ifdef USE_QT3D
qwtwc_API 	void qwtfigure3d(int n);
#endif

#ifdef USEMATHGL
/** add new MathGL plot.
 * \param n plot ID
 * */
qwtwc_API 	void qwtmgl(int n);

/**  Add 3D line.
 * \param style if 0, then use something default
 * */
qwtwc_API 	void qwtmgl_line(int size, double* x, double* y, double* z, const char* name, const char* style);

/** Add 3D mesh.
 *  \param data double array [xSize x ySize]; point M(x[i], y[j]) have data[i + xSize*j]
 * 
 *  \param type 0 - use 'mesh'; 1 - use 'surf'
 * */
qwtwc_API 	void qwtmgl_mesh(int xSize, int ySize, 
		double xMin, double xMax, double yMin, double yMax, 
		double* data, 
        const char* name,
		const char* style,
		int type);
#endif

/** put a title on currently 'active' plot.
	@param[in] s the title
*/
qwtwc_API 	void qwttitle(const char* s);

/** put a 'label' on X (bottom) axis.
	@param[s] axis name string
*/
qwtwc_API 	void qwtxlabel(const char* s);

/** put a 'label' on Y (left) axis.
@param[s] axis name string
*/
qwtwc_API 	void qwtylabel(const char* s);

/** close all figures.
*/
qwtwc_API 	void qwtclear();

/** This function sets some additional flags on (following) lines.
	@param[in] status if '0', all next lines will be "not important":
	 this means that they will not participate in 'clipping' and after pressing "clip" button
	 thier range will not be considered.
	 '1' would return this backward.
*/
qwtwc_API 	void qwtsetimpstatus(int status);

/**  add line to the current 'active' plot.
 should 'x' and 'y' arrays exists after call to this function? I hope no. 
 @param[in] x pointer to x values
 @param[in] y pointer to y-axis values
 @param[in] size size of 'x' and 'y' array 
 @param[in] name name for this particular line
 @param[in] style line style. This is a string with 1 or 2  or 3 char:

 \code
    string, 1, 2 or 3 characters;

      last char is always color:
      'r'  red
      'd'  darkRed
      'k'  black
      'w'  white (quite useless because background is white)
      'g'  green
      'G'  darkGreen
      'm'  magenta
      'M'  darkMagenta
      'y'  yellow
      'Y'  darkYellow
      'b'  blue
      'c'  cyan
      'C'  darkCyan

      //first char is always a line style:

      ' ' NoCurve
      '-' Lines
      '%' Sticks
      '#' Steps
      '.' Dots

       //   middle char is symbol type (if we need symbols):

       'e' Ellipse
       'r' Rect
       'd' Diamond
       't' Triangle
       'x' Cross
       's' Star1
       'q' Star2
       'w' XCross
       'u' UTriangle

 \endcode


 @param[in] lineWidth line width (1 is fastest)
 @param[in] symSize  size parameter for the symbols (if we use symbols instead of lines).
 \return this line ID

*/
qwtwc_API 	int qwtplot(double* x, double* y, int size, const char* name, const char* style, 
    int lineWidth, int symSize);

/**  same as above, but with additional 'time information'.
	\see qwtplot function description
	@param[in] time time info; we use this info to properly draw a marker on this plot.

	For example, we can create two plots describing some trajectory: on the first plot we output 'velocity vs time' info,
	and on second plot we output 'view from the top'; imagine we have following arrays for this trajectory:
	x coord
	y coord
	time
	x velo
	

	for first plot we'll use 'x velo' and 'time' arrays, and for second plot we'll use 'x coord', 'y coord' and 'time' arrays.

 \return this line ID
*/
qwtwc_API 	int qwtplot2(double* x, double* y, int size, const char* name, const char* style, 
    int lineWidth, int symSize, double* time);

/**
    remove plot with this particular ID
    \param id the value came from functions like qwtplot..;   the ID of the line to remove
*/
qwtwc_API void qwtremove(int id);

#ifdef USE_QT3D
qwtwc_API 	void qwtplot3d(double* x, double* y, double* z, int size, const char* name, const char* style,
	int lineWidth, int symSize, double* time);
#endif
#ifdef ENABLE_UDP_SYNC
qwtwc_API 	void qwtEnableCoordBroadcast(double* x, double* y, double* z, double* time, int size);
qwtwc_API 	void qwtDisableCoordBroadcast();
#endif

/** close QT proc (why not?  You do not have to, though..). 
 *  There is another function to close all windows.
*/
qwtwc_API 	void qwtclose(); 

/** Show 'main window' which allow to easily switch between other windows.

*/
qwtwc_API 	void qwtshowmw(); 


#ifdef __cplusplus
	}
#endif




