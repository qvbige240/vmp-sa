#ifndef UPNPGLOBAL_H
#define UPNPGLOBAL_H

/*!
 * \file
 *
 * \brief Defines constants that for some reason are not defined on some systems.
 */

#if defined MYLIB_LARGEFILE_SENSITIVE && _FILE_OFFSET_BITS+0 != 64
	#if defined __GNUC__
		#warning libupnp requires largefile mode - use AC_SYS_LARGEFILE
	#else
		#error  libupnp requires largefile mode - use AC_SYS_LARGEFILE
	#endif
#endif

#ifdef WIN32
	/*
	 * EXPORT_SPEC
	 */
	#ifdef UPNP_STATIC_LIB
		#define EXPORT_SPEC
	#else /* UPNP_STATIC_LIB */
		#ifdef LIBUPNP_EXPORTS
			/*! set up declspec for dll export to make functions
			 * visible to library users */
			#define EXPORT_SPEC __declspec(dllexport)
		#else /* LIBUPNP_EXPORTS */
			#define EXPORT_SPEC __declspec(dllimport)
		#endif /* LIBUPNP_EXPORTS */
	#endif /* UPNP_STATIC_LIB */

	/*
	 * UPNP_INLINE
	 * PRId64
	 * PRIzd
	 * PRIzu
	 * PRIzx
	 */
	#ifdef UPNP_USE_MSVCPP
		/* define some things the M$ VC++ doesn't know */
		#define UPNP_INLINE _inline
		typedef __int64 int64_t;
		#define PRId64 "I64d"
		#define PRIzd "ld"
		#define PRIzu "lu"
		#define PRIzx "lx"
	#endif /* UPNP_USE_MSVCPP */

	#ifdef UPNP_USE_BCBPP
		/* define some things Borland Builder doesn't know */
		#define UPNP_INLINE inline
		typedef __int64 int64_t;
		#warning The Borland C compiler is probably broken on PRId64,
		#warning please someone provide a proper fix here
		#define PRId64 "I64d"
		#define PRIzd "zd"
		#define PRIzu "zu"
		#define PRIzx "zx"
	#endif /* UPNP_USE_BCBPP */

	#ifdef __GNUC__
		#define UPNP_INLINE inline
		/* Note with PRIzu that in the case of Mingw32, it's the MS C
		 * runtime printf which ends up getting called, not the glibc
		 * printf, so it genuinely doesn't have "zu"
		 */
		#define PRIzd "ld"
		#define PRIzu "lu"
		#define PRIzx "lx"
	#endif /* __GNUC__ */
#else
	/*! 
	 * \brief Export functions on WIN32 DLLs.
	 *
	 * Every funtion that belongs to the library API must use this
	 * definition upon declaration or it will not be exported on WIN32
	 * DLLs.
	 */
	#define EXPORT_SPEC

	/*!
	 * \brief Declares an inline function.
	 *
	 * Surprisingly, there are some compilers that do not understand the
	 * inline keyword. This definition makes the use of this keyword
	 * portable to these systems.
	 */
	#ifdef __STRICT_ANSI__
		#define UPNP_INLINE __inline__
	#else
		#define UPNP_INLINE inline
	#endif

	/*!
	 * \brief Supply the PRId64 printf() macro.
	 *
	 * MSVC still does not know about this.
	 */
	/* #define PRId64 PRId64 */

	/*!
	 * \brief Supply the PRIz* printf() macros.
	 *
	 * These macros were invented so that we can live a little longer with
	 * MSVC lack of C99. "z" is the correct printf() size specifier for
	 * the size_t type.
	 */
	#define PRIzd "zd"
	#define PRIzu "zu"
	#define PRIzx "zx"
#endif

/*
 * Defining this macro here gives some interesting information about unused
 * functions in the code. Of course, this should never go uncommented on a
 * release.
 */
/*#define inline*/

#define STDC_HEADERS 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STRINGS_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1
#define HAVE_UNISTD_H 1
#define HAVE_DLFCN_H 1
#define LT_OBJDIR ".libs/"
#define _LARGE_FILE_SOURCE /**/
#define _FILE_OFFSET_BITS 64
#define STDC_HEADERS 1
#define HAVE_ARPA_INET_H 1
#define HAVE_FCNTL_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_NETDB_H 1
#define HAVE_NETINET_IN_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_IOCTL_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYSLOG_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_VPRINTF 1
#define HAVE_FSEEKO 1
#define HAVE_STRNLEN 1
#define HAVE_STRNDUP 1
#define HAVE_PTHREAD 1
#define UPNP_USE_RWLOCK 1

#endif /* UPNPGLOBAL_H */
