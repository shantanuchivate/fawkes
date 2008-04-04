
/***************************************************************************
 *  factory.cpp - Camera factory
 *
 *  Created: Wed Apr 11 15:37:45 2007
 *  Copyright  2005-2007  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. A runtime exception applies to
 *  this software (see LICENSE file mentioned below for details).
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You can read the full text in the LICENSE file in the doc directory. 
 */

#include <cams/factory.h>
#include <fvutils/system/camargp.h>

#ifdef HAVE_FIREWIRE_CAM
#include <cams/firewire.h>
#endif
#ifdef HAVE_LEUTRON_CAM
#include <cams/leutron.h>
#endif
#ifdef HAVE_FILELOADER_CAM
#include <cams/fileloader.h>
#endif
#ifdef HAVE_SHMEM_CAM
#include <cams/shmem.h>
#endif
#ifdef HAVE_NETWORK_CAM
#include <cams/net.h>
#endif
#ifdef HAVE_V4L_CAM
#include <cams/v4l.h>
#endif
#ifdef HAVE_BUMBLEBEE2_CAM
#include <cams/bumblebee2.h>
#endif

using namespace std;

/** @class UnknownCameraTypeException <cams/factory.h>
 * Unknown camera exception.
 * Thrown if the requested camera has not been recognized or the needed
 * libraries were not available at compile time.
 */

/** Constructor.
 * @param msg optional extra message
 */
UnknownCameraTypeException::UnknownCameraTypeException(const char *msg)
  : Exception("Unknown camera type")
{
  append(msg);
}


/** @class CameraFactory <cams/factory.h>
 * Camera factory.
 * This camera factory provides access to all cameras in a unified way. You just
 * supply a camera argument string and depending on the camera ID and compile-time
 * support of camera types an instance of the desired camera is returned or otherwise
 * an exception is thrown. See instance() for a list of supported camera types.
 *
 * @author Tim Niemueller
 */

/** Get camera instance with parameters from given camera argument parser.
 * This is a convenience method and works like instace(const char *as).
 * @param cap camera argument parser
 * @return camera instance
 * @exception UnknownCameraTypeException thrown if camera type is not known or
 * was not available at build time.
 */
Camera *
CameraFactory::instance(const CameraArgumentParser *cap)
{
  Camera *c = NULL;

  // ######
  if ( cap->cam_type() == "firewire" ) {
#ifdef HAVE_FIREWIRE_CAM
    c = new FirewireCamera(cap);
#else
    throw UnknownCameraTypeException("No firewire support at compile time");
#endif
  }

  // ######
  if ( cap->cam_type() == "leutron" ) {
#ifdef HAVE_LEUTRON_CAM
    c = new LeutronCamera();
#else
    throw UnknownCameraTypeException("No Leutron support at compile time");
#endif
  }

  // ######
  if ( cap->cam_type() == "file" ) {
#ifdef HAVE_FILELOADER_CAM
    c = new FileLoader(cap);
#else
    throw UnknownCameraTypeException("No file loader support at compile time");
#endif
  }

  // ######
  if ( cap->cam_type() == "shmem" ) {
#ifdef HAVE_SHMEM_CAM
    c = new SharedMemoryCamera(cap);
#else
    throw UnknownCameraTypeException("No shared memory support at compile time");
#endif
  }

  // ######
  if ( cap->cam_type() == "net" ) {
#ifdef HAVE_NETWORK_CAM
    c = new NetworkCamera(cap);
#else
    throw UnknownCameraTypeException("No network support at compile time");
#endif
  }

  // ######
  if ( cap->cam_type() == "v4l" ) {
#ifdef HAVE_V4L_CAM
    c = new V4LCamera(cap);
#else
    throw UnknownCameraTypeException("No video4linux support at compile time");
#endif
  }

  // ######
  if ( cap->cam_type() == "bumblebee2" ) {
#ifdef HAVE_BUMBLEBEE2_CAM
    c = new Bumblebee2Camera(cap);
#else
    throw UnknownCameraTypeException("No Bumblebee 2 support at compile time");
#endif
  }

  if ( c == NULL ) {
    throw UnknownCameraTypeException();
  }

  return c;
}


/** Get camera instance.
 * Get an instance of a camera of the given type. The argument string determines
 * the type of camera to open.
 * Supported camera types:
 * - firewire, FirewireCamera, compiled if HAVE_FIREWIRE_CAM is defined in fvconf.mk
 * - leutron, LeutronCamera, compiled if HAVE_LEUTRON_CAM is defined in fvconf.mk
 * - file, FileLoader, compiled if HAVE_FILELOADER_CAM is defined in fvconf.mk
 * - shmem, SharedMemoryCamera, compiled if HAVE_SHMEM_CAM is defined in fvconf.mk
 * - net, NetworkCamera, compiled if HAVE_NETWORK_CAM is defined in fvconf.mk
 * - v4l, V4LCamera, compiled if HAVE_V4L_CAM is defined in fvconf.mk
 * @param as camera argument string
 * @return camera instance of requested type
 * @exception UnknownCameraTypeException thrown, if the desired camera could
 * not be instantiated. This could be either to a misspelled camera ID, generally
 * missing support or unset definition due to configuration in fvconf.mk or missing
 * libraries and camera support compile-time autodetection.
 */
Camera *
CameraFactory::instance(const char *as)
{
  CameraArgumentParser *cap = new CameraArgumentParser(as);
  try {
    Camera *cam = instance(cap);
    delete cap;
    return cam;
  } catch (UnknownCameraTypeException &e) {
    delete cap;
    throw;
  }
}
