# From: http://www.openlibraries.org/browser/trunk/FindFFMPEG.cmake

# - Try to find FFMPEG
# Once done this will define
#
#  FFMPEG_FOUND -         system has FFMPEG
#  FFMPEG_INCLUDE_DIR -   the include directories
#  FFMPEG_LIBRARY_DIR -   the directory containing the libraries
#  FFMPEG_LIBRARIES -     link these to use FFMPEG
#  FFMPEG_SWSCALE_FOUND - FFMPEG also has SWSCALE
#

#SET( FFMPEG_HEADERS avformat.h avcodec.h avutil.h avdevice.h )
#SET( FFMPEG_PATH_SUFFIXES libavformat libavcodec libavutil libavdevice )
SET( FFMPEG_HEADERS avformat.h avcodec.h avutil.h )
SET( FFMPEG_PATH_SUFFIXES libavformat libavcodec libavutil )
SET( FFMPEG_SWS_HEADERS swscale.h )
SET( FFMPEG_SWS_PATH_SUFFIXES libswscale )

SET( FFMPEG_LIBRARY_DIR $ENV{FFMPEGDIR}/lib )

if( WIN32 )
   #SET( FFMPEG_LIBRARIES avformat.lib avcodec.lib avutil.lib avdevice.lib )
   SET( FFMPEG_LIBRARIES avformat.lib avcodec.lib avutil.lib )
   SET( FFMPEG_SWS_LIBRARIES swscale.lib )
   SET( FFMPEG_LIBRARY_DIR $ENV{FFMPEGDIR}\\lib )
   SET( FFMPEG_INCLUDE_PATHS $ENV{FFMPEGDIR}\\include )

   # check to see if we can find swscale
   SET( TMP_ TMP-NOTFOUND )
   FIND_PATH( TMP_ ${FFMPEG_SWS_LIBRARIES}
              PATHS ${FFMPEG_LIBRARY_DIR} )
   IF ( TMP_ )
      SET( SWSCALE_FOUND TRUE )
   ENDIF( TMP_ )
else( WIN32 )
   #SET( FFMPEG_LIBRARIES avformat avcodec avutil avdevice )
   SET( FFMPEG_LIBRARIES avformat avcodec avutil )
   SET( FFMPEG_SWS_LIBRARIES swscale )
   INCLUDE(FindPkgConfig)
   if ( PKG_CONFIG_FOUND )
      pkg_check_modules( AVFORMAT libavformat )
      pkg_check_modules( AVCODEC libavcodec )
      pkg_check_modules( AVUTIL libavutil )
      #pkg_check_modules( AVDEVICE libavdevice )
      pkg_check_modules( SWSCALE libswscale )
   endif ( PKG_CONFIG_FOUND )

   SET( FFMPEG_LIBRARY_DIR   ${AVFORMAT_LIBRARY_DIRS}
                             ${AVCODEC_LIBRARY_DIRS}
                             ${AVUTIL_LIBRARY_DIRS}
                             #${AVDEVICE_LIBRARY_DIRS}
                             )
   SET( FFMPEG_INCLUDE_PATHS ${AVFORMAT_INCLUDE_DIRS}
                             ${AVCODEC_INCLUDE_DIRS}
                             ${AVUTIL_INCLUDE_DIRS}
                             #${AVDEVICE_INCLUDE_DIRS}
                             )
   # check to see if we can find swscale
   FIND_LIBRARY(LIB_SWSCALE_ swscale ${FFMPEG_LIBRARY_DIR} )
   IF ( LIB_SWSCALE_ )
      SET( SWSCALE_FOUND TRUE )
   ENDIF( LIB_SWSCALE_ )

endif( WIN32 )


# add in swscale if found
IF ( SWSCALE_FOUND )
   SET( FFMPEG_LIBRARY_DIR   ${FFMPEG_LIBRARY_DIR}
                             ${SWSCALE_LIBRARY_DIRS} )
   SET( FFMPEG_INCLUDE_PATHS ${FFMPEG_INCLUDE_PATHS}
                             ${SWSCALE_INCLUDE_DIRS} )
   SET( FFMPEG_HEADERS       ${FFMPEG_HEADERS}
                             ${FFMPEG_SWS_HEADERS} )
   SET( FFMPEG_PATH_SUFFIXES ${FFMPEG_PATH_SUFFIXES}
                             ${FFMPEG_SWS_PATH_SUFFIXES} )
   SET( FFMPEG_LIBRARIES     ${FFMPEG_LIBRARIES}
                             ${FFMPEG_SWS_LIBRARIES} )
ENDIF ( SWSCALE_FOUND )

# find includes
SET( INC_SUCCESS 0 )
SET( TMP_ TMP-NOTFOUND )
SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_PATHS} )
FOREACH( INC_ ${FFMPEG_HEADERS} )
   message(STATUS "checking: ${INC_}" )

   FIND_PATH( TMP_ ${INC_}
              PATHS ${FFMPEG_INCLUDE_PATHS}
              PATH_SUFFIXES ${FFMPEG_PATH_SUFFIXES} )
   IF ( TMP_ )
      message(STATUS "          ${TMP_}" )
      MATH( EXPR INC_SUCCESS ${INC_SUCCESS}+1 )
      SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} ${TMP_} )
   ENDIF ( TMP_ )
   SET( TMP_ TMP-NOTFOUND )
ENDFOREACH( INC_ )

list(LENGTH FFMPEG_INCLUDE_DIR LENGTH_INCLUDES)
list(LENGTH FFMPEG_LIBRARY_DIR LENGTH_LIBRARIES)

# clear out duplicates
if(LENGTH_INCLUDES GREATER 0)
LIST( REMOVE_DUPLICATES FFMPEG_INCLUDE_DIR )
endif(LENGTH_INCLUDES GREATER 0)
if(LENGTH_LIBRARIES GREATER 0)
LIST( REMOVE_DUPLICATES FFMPEG_LIBRARY_DIR )
endif(LENGTH_LIBRARIES GREATER 0)

# find the full paths of the libraries
SET( TMP_ TMP-NOTFOUND )
IF ( NOT WIN32 )
   FOREACH( LIB_ ${FFMPEG_LIBRARIES} )
      FIND_LIBRARY( TMP_ NAMES ${LIB_} PATHS ${FFMPEG_LIBRARY_DIR} )
      IF ( TMP_ )
         SET( FFMPEG_LIBRARIES_FULL ${FFMPEG_LIBRARIES_FULL} ${TMP_} )
      ENDIF ( TMP_ )
      SET( TMP_ TMP-NOTFOUND )
   ENDFOREACH( LIB_ )
   SET ( FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES_FULL} )
ENDIF( NOT WIN32 )

LIST( LENGTH FFMPEG_HEADERS LIST_SIZE_ )

SET( FFMPEG_FOUND FALSE )
SET( FFMPEG_SWSCALE_FOUND FALSE )
IF ( ${INC_SUCCESS} EQUAL ${LIST_SIZE_} )
   SET( FFMPEG_FOUND TRUE )
   SET( FFMPEG_SWSCALE_FOUND ${SWSCALE_FOUND} )
ENDIF ( ${INC_SUCCESS} EQUAL ${LIST_SIZE_} )

string(REPLACE "/usr/include/" "" FFMPEG_INCLUDE_DIR "${FFMPEG_INCLUDE_DIR}")

# add libx264 ...
if (APPLE)
FIND_LIBRARY(LIB_X264 NAMES x264 libx264.a PATHS /usr/local/lib $ENV{LIBX264DIR}/lib DOC "x264 library" )
#SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} "/Users/val/Documents/Sources/slowlib/lib/libx264.a" )
SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${LIB_X264} )
message(STATUS "x264 found in: ${LIB_X264}" )
endif()

# On OS X we ffmpeg libraries depend on VideoDecodeAcceleration and CoreVideo frameworks
IF (APPLE)
    SET(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} "-framework CoreFoundation -framework QuartzCore -framework VideoDecodeAcceleration -liconv -lbz2 -lz")
ENDIF()


