//ifdef _MSC_VER    = Microsoft Compiler
//ifdef __MINGW32__ = MinGW Compiler (both)
//ifdef __MINGW64__ = MinGW 64-bit Compiler
#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_WINDOWS
#define WINVER         0x0502  // Windows XP SP2 & Windows Server 2003 SP1 or greater
#define _WIN32_WINNT   0x0502
#define _WIN32_WINDOWS 0x0502
//#undef NTDDI_VERSION
//#define NTDDI_VERSION 0x05020000  // Windows Server 2003 or greater

// exclude MFC stuff from headers
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRA_LEAN
#define WIN32_EXTRA_LEAN

#define _CRT_NON_CONFORMING_SWPRINTFS
#ifndef __MINGW32__  // Microsoft C
//#pragma warning (disable:4995)  // W3: name was marked as #pragma deprecated
//#pragma warning (disable:4996)  // W3: deprecated declaration (_CRT_SECURE_NO_WARNINGS)
#pragma warning (disable:4711)  // W1: function selected for inline expansion
#pragma warning (disable:4710)  // W4: function not inlined
//#pragma warning (disable:4127)  // W4: conditional expression is constant
//#pragma warning (disable:4820)  // W4: bytes padding added
//#pragma warning (disable:4668)  // W4: symbol not defined as a macro
//#pragma warning (disable:4255)  // W4: no function prototype given
#pragma warning (disable:4100)  // W4: unreferenced formal parameter
//#pragma warning (disable:4101)  // W3: unreferenced local variable
#pragma warning (disable:5045)  // W3: Spectre mitigation

#else // MinGW32
//#pragma GCC diagnostic ignored "-Wunknown-pragmas"
//#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
//#pragma GCC diagnostic ignored "-Wunused-result"
//#pragma GCC diagnostic ignored "-Wwrite-strings"
//#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#ifdef __MINGW64__  // MinGW-w64 64-bit
int _dowildcard = 0;  // disable command line globbing
#else
int _CRT_glob = 0;  // disable command line globbing
#endif
#endif

#undef _MBCS
#define UNICODE   // for ...W function calls
#define _UNICODE  // for _TCHAR support
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <sys/stat.h>
#include <shellapi.h>

#define SW_VERSION _T("1.02")
#define SW_BUILD   _T("Aug 25, 2022")

#define DIRSEP _T('\\')
#define DIRSES _T("\\")
#define NULLC  _T('\0')
#define CTRL_Z 26

#ifdef _UNICODE
#define PS _T("%ls")
#else
#define PS _T("%s")
#endif

#define MAXS 1024
#define MAXL 10240
#define MAXP 32767
#define MAXC 5000

#ifndef QWORD
typedef unsigned long long QWORD;
#endif

// global variables
_TCHAR *prog;  // program name
int debug, quiet, all, exist;
int final_ok, final_err;


// ----------------------------------------------------------------------------
// xstrncpy
// return s1 (= s2)
// ----------------------------------------------------------------------------
_TCHAR* xstrncpy(_TCHAR *s1, const _TCHAR *s2, size_t n)
{
  size_t ii;

  if (s1 != NULL && s2 != NULL) {
    // copy until length of s1 == n or s2 is empty
    for (ii = 0; ii < n && *s2; ii++)
      *s1++ = *s2++;
    *s1 = NULLC;
  }

  return s1;
} /* xstrncpy */


// ----------------------------------------------------------------------------
// xstrncat
// return s1 (= s1 + s2)
// ----------------------------------------------------------------------------
_TCHAR* xstrncat(_TCHAR *s1, const _TCHAR *s2, size_t n)
{
  size_t ii;

  if (s1 != NULL && s2 != NULL) {
    for (ii = 0; ii < n && *s1; ii++) s1++;
    /* copy until length of s1 == n or s2 is empty */
    for ( ; ii < n && *s2; ii++)
      *s1++ = *s2++;
    *s1 = NULLC;
  }

  return(s1);
} /* xstrncat */


// ----------------------------------------------------------------------------
// xstrtrim
// return ptr_str, trimmed from beginning (str trimmed at end)
// ----------------------------------------------------------------------------
_TCHAR* xstrtrim(_TCHAR *str)
{
  _TCHAR *beg;
  int ii;

  if (str == NULL) return NULL;

  // trim white spaces at the end
  ii = (int)_tcslen(str) - 1;
  while (ii >= 0 && _istspace(str[ii]))
    str[ii--] = NULLC;

  // trim white spaces at the beginning
  beg = str;
  while (_istspace(*beg)) beg++;

  return beg;
} /* xstrtrim */


// ----------------------------------------------------------------------------
// xstrsep
// return ptr_token from *strp, terminated with \0 or NULL if no tokens anymore
// ----------------------------------------------------------------------------
_TCHAR* xstrsep(_TCHAR **strp, const _TCHAR *delim)
{
  _TCHAR *str, *s;
  int found;

  if (strp == NULL || *strp == NULL) return NULL;
  str = *strp;
  if (delim == NULL) return str;

  for (found = 0; **strp && !found; (*strp)++) {
    for (s = (_TCHAR *)delim; *s && !found; s++) {
      if (**strp == *s) {
        **strp = NULLC;
        found = 1;
      }
    }
  }
  if (!found) *strp = NULL;

  return str;
} /* xstrsep */


// ----------------------------------------------------------------------------
// xstrstr (Boyer-Moore-Horspool)
// return ptr_s1, where s2 is found or NULL if s2 is not found
// ----------------------------------------------------------------------------
#ifdef _UNICODE
#define BMH 0xFFFF
#else
#define BMH 0xFF
#endif

_TCHAR* xstrstr(_TCHAR *s1, const _TCHAR *s2)
{
  size_t n1, n2, ii, last;
  size_t skip[BMH];

  if (s1 == NULL || s2 == NULL) return NULL;
  n1 = _tcslen(s1);
  n2 = _tcslen(s2);
  if (n2 > n1) return NULL;
  if (n2 == 0) return s1;

  for (ii = 0; ii < BMH; ii++)
    skip[ii] = n2;

  last = n2 - 1;

  for (ii = 0; ii < last; ii++)
    skip[s2[ii] & BMH] = last - ii;

  while (n1 >= n2) {
    for (ii = last; s1[ii] == s2[ii]; ii--)
      if (ii == 0) return s1;

    n1 -= skip[s1[last] & BMH];
    s1 += skip[s1[last] & BMH];
  }

  return NULL;
} /* xstrstr */


// ----------------------------------------------------------------------------
// xstrcasestr (case insensitive Boyer-Moore-Horspool)
// return ptr_s1, where s2 is found or NULL if s2 is not found
// ----------------------------------------------------------------------------
#ifdef _UNICODE
#define BMH 0xFFFF
#else
#define BMH 0xFF
#endif

_TCHAR* xstrcasestr(_TCHAR *s1, const _TCHAR *s2)
{
  size_t n1, n2, ii, last;
  size_t skip[BMH];
  _TCHAR *us2;

  if (s1 == NULL || s2 == NULL) return NULL;
  n1 = _tcslen(s1);
  n2 = _tcslen(s2);
  if (n2 > n1) return NULL;
  if (n2 == 0) return s1;

  for (ii = 0; ii < BMH; ii++)
    skip[ii] = n2;

  last = n2 - 1;

  us2 = (_TCHAR *)malloc(n2 * sizeof(_TCHAR));
  if (us2 == NULL) return NULL;
  for (ii = 0; ii < n2; ii++)
    us2[ii] = _totupper(s2[ii]);

  for (ii = 0; ii < last; ii++) {
    skip[us2[ii] & BMH] = last - ii;
    skip[_totlower(us2[ii]) & BMH] = last - ii;
  }

  while (n1 >= n2) {
    for (ii = last; _totupper(s1[ii]) == us2[ii]; ii--)
      if (ii == 0) { free(us2); return s1; }

    n1 -= skip[s1[last] & BMH];
    s1 += skip[s1[last] & BMH];
  }

  free(us2);
  return NULL;
} /* xstrcasestr */


// ----------------------------------------------------------------------------
// xstrerror
// return expanded error message (for specified errno)
// ----------------------------------------------------------------------------
_TCHAR* xstrerror(DWORD errnum)
{
  static _TCHAR msg[MAXL+1];
  int len;

//errnum = GetLastError();
  *msg = NULLC;
  FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, errnum,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // default language
    (LPTSTR)&msg, MAXL, NULL);

  // trim whitespaces from the end
  len = (int)_tcslen(msg) - 1;
  while (len >= 0 && (_istspace(msg[len]) || msg[len] == _T('.')))
    msg[len--] = NULLC;

  return msg;
} /* xstrerror */


// ----------------------------------------------------------------------------
// xflush
// flush stdin (= console input buffer)
// ----------------------------------------------------------------------------
void xflush(void)
{
  HANDLE stdinHandle;
#if 0
  int rc, error;
#endif

  stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
  if (stdinHandle == INVALID_HANDLE_VALUE) {
#if 0
    error = GetLastError();
    _ftprintf(stderr, _T("GetStdHandle: "PS" (%lu)\n"), xstrerror(error), error);
#endif
    return;
  }
#if 0
  rc = FlushConsoleInputBuffer(stdinHandle);
  if (rc == 0) {
    error = GetLastError();
    _ftprintf(stderr, _T("FlushConsoleInputBuffer: "PS" (%lu)\n"), xstrerror(error), error);
  }
#else
  FlushConsoleInputBuffer(stdinHandle);
#endif
} /* xflush */


// ----------------------------------------------------------------------------
// xgetline
// read entire line from stream, stored in lineptr (n reflects allocated size)
// lineptr must be freed afterwards!
// ----------------------------------------------------------------------------
int xgetline(_TCHAR **lineptr, int *n, FILE *stream)
{
  int pos, c;

  if (lineptr == NULL || n == NULL || stream == NULL) {
    SetLastError(ERROR_BAD_ARGUMENTS);
    errno = EINVAL;
    return -1;
  }

  c = getc(stream);
  //if (debug) { _tprintf(_T("Got char: %d |%c|\n"), c, c); fflush(NULL); }
  if (c == EOF) {
    SetLastError(NO_ERROR);
    errno = 0;
    return -1;
  }

  if (*lineptr == NULL) {
    *lineptr = (_TCHAR *)malloc(MAXS * sizeof(_TCHAR));
    if (*lineptr == NULL) {
      SetLastError(ERROR_OUTOFMEMORY);
      errno = ENOMEM;
      return -1;
    }
    *n = MAXS;
  }

  pos = 0;
  while (c != EOF && c != CTRL_Z) {
    if (pos + 1 >= *n) {
      int new_size = *n + (*n >> 1);
      if (new_size < MAXS) new_size = MAXS;
      _TCHAR *new_ptr = (_TCHAR *)realloc(*lineptr, (new_size * sizeof(_TCHAR)));
      if (new_ptr == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        errno = ENOMEM;
        return -1;
      }
      *lineptr = new_ptr;
      *n = new_size;
    }

    (*lineptr)[pos++] = (_TCHAR)c;
    if (c == _T('\n')) break;

    c = getc(stream);
    //if (debug) { _tprintf(_T("Got char: %d |%c|\n"), c, c); fflush(NULL); }
  }
  (*lineptr)[pos] = NULLC;

  SetLastError(NO_ERROR);
  errno = 0;
  return pos;
} /* xgetline */

// ----------------------------------------------------------------------------
// print_version
// ----------------------------------------------------------------------------
void print_version(void)
{
  _tprintf(_T(""PS" "PS"  Copyright (c) 2022 Matjaz Rihtar  ("PS")\n"),
           prog, SW_VERSION, SW_BUILD);
} /* print_version */


// ----------------------------------------------------------------------------
// print_usage
// ----------------------------------------------------------------------------
void print_usage(void)
{
  _tprintf(_T("Checks if file/directory exists, regardless of attributes.\n\n"));
  _tprintf(_T("Usage: "PS" [-q] [-a] [-e|-n] name ...\n\n"), prog);
  _tprintf(_T("  -q    quiet: don't show any error text\n"));
  _tprintf(_T("  -a    all: all specified names must exist or not\n"));
  _tprintf(_T("  -e    exist: returns 0 if name exist (default)\n"));
  _tprintf(_T("  -n    doesn't exist: returns 0 if name doesn't exist\n"));
//_tprintf(_T("  -d    debug: print additional debug information\n"));
  _tprintf(_T("  -v    version: prints program version and exits\n\n"));
  _tprintf(_T("Returns 0 if file/directory exist or > 0 if an error occured.\n"));
} /* print_usage */


// ----------------------------------------------------------------------------
// main (mingw32 unicode wrapper)
// ----------------------------------------------------------------------------
#if defined(_UNICODE) && defined(__MINGW32__)
#undef _tmain
#define _tmain wmain

extern int _CRT_glob;
extern
#ifdef __cplusplus
"C"
#endif
void __wgetmainargs(int*, wchar_t***, wchar_t***, int, int*);

int wmain(int argc, wchar_t *argv[], wchar_t *envp[]);

int main() {
  wchar_t **argv, **envp;
  int argc, si = 0;

  __wgetmainargs(&argc, &argv, &envp, _CRT_glob, &si);
  return wmain(argc, argv, envp);
}
#endif

// ----------------------------------------------------------------------------
// _tmain
// ----------------------------------------------------------------------------
int _tmain(int argc, _TCHAR *argv[], _TCHAR *envp[])
{
  int ii, ac, opt, rc;
  unsigned int error;
  _TCHAR *s, *av[MAXC];
  _TCHAR fpname[MAXP+1], *fnp;
  struct _stat64 sb;

  // get program name
  if ((prog = _tcsrchr(argv[0], DIRSEP)) == NULL) prog = argv[0];
  else prog++;
  if ((s = xstrcasestr(prog, _T(".exe"))) != NULL) *s = NULLC;
  for (ii = 0; ii < (int)_tcslen(prog); ii++)
    prog[ii] = _totupper(prog[ii]);

  // default global flags
  debug = 0;  // no debug
  quiet = 0;
  all = 0;
  exist = 1;

  // parse command line
  ac = 0; opt = 1;
  for (ii = 1; ii < argc && ac < MAXC; ii++) {
    if (opt && *argv[ii] == _T('-')) {
      if (_tcsicmp(argv[ii], _T("--")) == 0) {  // end of options
        opt = 0;
        continue;
      }
      s = argv[ii];
      while (*++s) {
        switch (_totlower(*s)) {
          case _T('q'):  // quiet
            quiet = 1;
            break;
          case _T('a'):  // all
            all = 1;
            break;
          case _T('e'):  // exist (default)
            exist = 1;
            break;
          case _T('n'):  // doesn't exist
            exist = 0;
            break;
          case _T('d'):  // debug
            debug++;
            break;
          case _T('v'):  // version
            print_version();
            exit(0);
            break;
          default:
error:      print_version();
            print_usage();
            exit(2);
        } // switch
      } // while
      continue;
    } // if (opt && /)
    if (_tcslen(argv[ii]) > 0) {
      av[ac] = (_TCHAR *)malloc((MAXL+1) * sizeof(_TCHAR));
      if (av[ac] == NULL) {
        error = GetLastError();
        _ftprintf(stderr, _T("malloc: "PS" (%u)\n"), xstrerror(error), error);
        exit(11);
      }
      xstrncpy(av[ac++], argv[ii], MAXL);
    }
  } // for (argc)
  av[ac] = NULL;

  if (ac == 0) goto error;

  final_ok = 0; final_err = 0;
  for (ii = 0; ii < ac; ii++) {
    //if (debug) { _tprintf(_T("Parameter %d: |"PS"|\n"), ii, av[ii]); fflush(NULL); }
    rc = GetFullPathName(av[ii], MAXP, (LPTSTR)&fpname, &fnp);
    if (rc == 0) {
      error = GetLastError();
      if (!quiet)
        _ftprintf(stderr, _T(""PS": "PS" (%u)\n"), av[ii], xstrerror(error), error);
      final_err++;
      continue;
    }
    //if (debug) { _tprintf(_T("Full path name: |"PS"|, fnp: |"PS"|\n"), fpname, fnp); fflush(NULL); }

    // check if specified file/directory exists
    rc = _tstat64(fpname, &sb);
    if (rc < 0) {
      error = GetLastError();
      if (!quiet)
        _ftprintf(stderr, _T(""PS": "PS" (%u)\n"), fpname, xstrerror(error), error);
      final_err++;
      continue;
    }
    final_ok++;
  } // for (ac)

  if (!all) {
    if (final_ok) final_ok = ii;
    if (final_err) final_err = ii;
  }
  if (exist) {
    if (final_ok == ii) return 0;
    else return 1;
  }
  else {
    if (final_err == ii) return 0;
    else return 1;
  }
} /* main */
