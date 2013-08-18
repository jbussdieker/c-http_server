#!/bin/sh
warn() {
  echo "WARNING: $@" 1>&2
}

case `uname -s` in
  Darwin)
    LIBTOOLIZE=glibtoolize
  ;;
  FreeBSD)
    LIBTOOLIZE=libtoolize
  ;;
  OpenBSD)
    LIBTOOLIZE=libtoolize
  ;;
  Linux)
    LIBTOOLIZE=libtoolize
  ;;
  SunOS)
    LIBTOOLIZE=libtoolize
  ;;
  *)
    warn "unrecognized platform:" `uname -s`
    LIBTOOLIZE=libtoolize
  ;;
esac

set -ex
$LIBTOOLIZE --copy --force
aclocal -I m4
automake --add-missing --copy --foreign
autoconf
./configure $@
