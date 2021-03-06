dnl $Id$
dnl config.m4 for extension snuffleupagus

sources="snuffleupagus.c sp_config.c sp_config_utils.c sp_harden_rand.c"
sources="$sources sp_unserialize.c sp_utils.c sp_disable_xxe.c sp_list.c"
sources="$sources sp_disabled_functions.c sp_execute.c sp_upload_validation.c"
sources="$sources sp_cookie_encryption.c sp_network_utils.c tweetnacl.c"
sources="$sources sp_config_keywords.c sp_compile.c"

PHP_ARG_ENABLE(snuffleupagus, whether to enable snuffleupagus support,
[  --enable-snuffleupagus           Enable snuffleupagus support])

PHP_ARG_ENABLE(coverage, whether to enable coverage support,
[  --enable-coverage           Enable coverage support], no, no)

PHP_ARG_ENABLE(debug, whether to enable debug messages,
[  --enable-debug           Enable debug messages], no, no)

CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -D_DEFAULT_SOURCE=1 -std=c99"
CFLAGS="$CFLAGS -Wall -Wextra -Wno-unused-parameter"
CFLAGS="$CFLAGS -Wformat=2 -Wformat-security -D_FORTIFY_SOURCE=2"
CFLAGS="$CFLAGS -fstack-protector"

LDFLAGS="$LDFLAGS -lpcre"

if test "$PHP_DEBUG" = "yes"; then
	AC_DEFINE(SP_DEBUG, 1, [Wether you want to enable debug messages])
      CFLAGS="$CFLAGS -g -ggdb -O0"
fi

AC_CHECK_LIB(pcre, pcre_compile, AC_DEFINE(HAVE_PCRE, 1, [have pcre]))

if test "$PHP_SNUFFLEUPAGUS" = "yes"; then
   if test "$PHP_COVERAGE" = "yes"; then
      CFLAGS="$CFLAGS --coverage -O0 -g"
   fi
   PHP_NEW_EXTENSION(snuffleupagus, $sources, $ext_shared,-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
