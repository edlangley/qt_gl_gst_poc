# **************************************************************************
# SIM_AC_HAVE_SIMAGE_IFELSE( IF-FOUND, IF-NOT-FOUND )
#
# Description:
#   This macro locates the simage development system.  If it is found, the
#   set of variables listed below are set up as described and made available
#   to the configure script.
#
#   The $sim_ac_simage_desired variable can be set to false externally to
#   make SoXt default to be excluded.
#
# Autoconf Variables:
# > $sim_ac_simage_desired   true | false (defaults to true)
# < $sim_ac_simage_avail     true | false
# < $sim_ac_simage_cppflags  (extra flags the compiler needs for simage)
# < $sim_ac_simage_ldflags   (extra flags the linker needs for simage)
# < $sim_ac_simage_libs      (link libraries the linker needs for simage)
# < $sim_ac_simage_version   (the libsimage version)
#
# Authors:
#   Morten Eriksen <mortene@coin3d.org>
#   Lars J. Aas <larsa@coin3d.org>
#
# TODO:
# - rework variable name convention
# - clean up shell scripting redundancy
# - support debug symbols simage library
#

AC_DEFUN([SIM_AC_HAVE_SIMAGE_IFELSE],
[AC_PREREQ([2.14a])

# official variables
sim_ac_simage_avail=false
sim_ac_simage_cppflags=
sim_ac_simage_ldflags=
sim_ac_simage_libs=
sim_ac_simage_version=

# internal variables
: ${sim_ac_simage_desired=true}
sim_ac_simage_extrapath=

AC_ARG_WITH(
  simage,
  AC_HELP_STRING([--with-simage=DIR],
                 [use simage for loading texture files]),
  [case $withval in
   yes) sim_ac_simage_desired=true ;;
   no)  sim_ac_simage_desired=false ;;
   *)   sim_ac_simage_desired=true
        sim_ac_simage_extrapath=$withval ;;
  esac],
  [])

if [ $sim_ac_simage_desired = true ]; then
  sim_ac_path=$PATH
  test -z "$sim_ac_simage_extrapath" ||
    sim_ac_path=$sim_ac_simage_extrapath/bin:$sim_ac_path
  test x"$prefix" = xNONE ||
    sim_ac_path=$sim_ac_path:$prefix/bin

  AC_PATH_PROG(sim_ac_simage_configcmd, simage-config, false, $sim_ac_path)

  if $sim_ac_simage_configcmd; then
    test -n "$CONFIG" &&
      $sim_ac_simage_configcmd --alternate=$CONFIG >/dev/null 2>/dev/null &&
      sim_ac_simage_configcmd="$sim_ac_simage_configcmd --alternate=$CONFIG"
    sim_ac_simage_cppflags=`$sim_ac_simage_configcmd --cppflags`
    sim_ac_simage_ldflags=`$sim_ac_simage_configcmd --ldflags`
    sim_ac_simage_libs=`$sim_ac_simage_configcmd --libs`
    sim_ac_simage_version=`$sim_ac_simage_configcmd --version`
    AC_CACHE_CHECK([whether the simage library is available],
      sim_cv_simage_avail,
      [sim_ac_save_cppflags=$CPPFLAGS
      sim_ac_save_ldflags=$LDFLAGS
      sim_ac_save_libs=$LIBS
      CPPFLAGS="$CPPFLAGS $sim_ac_simage_cppflags"
      LDFLAGS="$LDFLAGS $sim_ac_simage_ldflags"
      LIBS="$sim_ac_simage_libs $LIBS"
      AC_TRY_LINK(
        [#include <simage.h>],
        [(void)simage_read_image(0L, 0L, 0L, 0L);],
        [sim_cv_simage_avail=true],
        [sim_cv_simage_avail=false])
      CPPFLAGS=$sim_ac_save_cppflags
      LDFLAGS=$sim_ac_save_ldflags
      LIBS=$sim_ac_save_libs
    ])
    sim_ac_simage_avail=$sim_cv_simage_avail
  else
    locations=`IFS=:
               for p in $sim_ac_path; do echo " -> $p/simage-config"; done`
    AC_MSG_WARN([cannot find 'simage-config' at any of these locations:
$locations])
  fi
fi

if $sim_ac_simage_avail; then
  ifelse([$1], , :, [$1])
else
  ifelse([$2], , :, [$2])
fi
])

# EOF **********************************************************************
