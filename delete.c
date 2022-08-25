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

#define SW_VERSION _T("1.11")
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

#define FILE_ATTRIBUTE_MASK ( \
  FILE_ATTRIBUTE_READONLY | \
  FILE_ATTRIBUTE_HIDDEN | \
  FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | \
  FILE_ATTRIBUTE_SYSTEM | \
  FILE_ATTRIBUTE_ARCHIVE | \
  FILE_ATTRIBUTE_REPARSE_POINT | \
  FILE_ATTRIBUTE_OFFLINE )

#ifndef QWORD
typedef unsigned long long QWORD;
#endif

// global variables
_TCHAR *prog;  // program name
int debug;
int prompt, force, subdirs, quiet;
unsigned int pos_attrs, neg_attrs;
int files_found;
int final_rc;


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
// getconf
// get confirmation for question (Y/N)
// ----------------------------------------------------------------------------
int getconf(_TCHAR *fname, _TCHAR *question)
{
  _TCHAR *line = NULL;
  int len, rlen;
  _TCHAR *ans, c;
  int conf = 0;

  xflush(); //fflush(stdin);
  while (1) {
    _ftprintf(stderr, _T(""PS", "PS" (Y/N)? "), fname, question);
    line = NULL; len = 0;
    rlen = xgetline(&line, &len, stdin);
    if (rlen >= 0) {
      ans = xstrtrim(line);
      //if (debug) { _tprintf(_T("Answer: |"PS"|\n"), ans); fflush(NULL); }
      c = _totupper(*ans);
      if (c == _T('Y') || c == _T('N')) {
        conf = c == _T('Y');
        break;
      }
    }
    else if (!errno) break;  // EOF
    if (line != NULL) free(line);
  }
  if (line != NULL) free(line);
  return conf;
} /* getconf */


// ----------------------------------------------------------------------------
// ShellDelete
// delete file/directory to Recycle Bin
// ----------------------------------------------------------------------------
int ShellDelete(_TCHAR *fname, int verbose)
{
  SHFILEOPSTRUCT shFileOp;
  _TCHAR *flist;
  unsigned int error;
  int rc;

  // last file name must be terminated with a double NULL character
  // (watch out for the threshold special characters at the end of buffer)
  flist = (_TCHAR *)calloc((_tcslen(fname) + 2), sizeof(_TCHAR));
  if (flist == NULL) {
    error = GetLastError();
    _ftprintf(stderr, _T("calloc: "PS" (%u)\n"), xstrerror(error), error);
    final_rc++;
    return 1;
  }
  xstrncpy(flist, fname, MAXP);

  memset(&shFileOp, 0, sizeof(shFileOp));
  shFileOp.hwnd = GetDesktopWindow();
  shFileOp.wFunc = FO_DELETE;
  shFileOp.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
  shFileOp.pFrom = flist;

  rc = SHFileOperation(&shFileOp);
  if (rc != 0) {
    error = GetLastError();
    _ftprintf(stderr, _T(""PS": "PS" (%u)\n"), fname, xstrerror(error), error);
    final_rc++;
  }
  else if (verbose)
    _tprintf(_T("Deleted file - "PS"\n"), fname);

  free(flist);
  return rc;
} /* ShellDelete */


// ----------------------------------------------------------------------------
// process
// search for and delete fname, recursively if needed
// ----------------------------------------------------------------------------
void process(_TCHAR *fdir, _TCHAR *fname)
{
  unsigned int error = ERROR_INVALID_DATA;
  HANDLE ff = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA fa;
  _TCHAR xdir[MAXP+1], xname[MAXP+1];
  int len, ask;
  int rc, ro_doit, hid_doit, sys_doit, ar_doit, nci_doit, rp_doit, off_doit;
  int ro_fail, pos_doit, neg_doit, doit, conf;
  unsigned int flags;
  DWORD fa_attrs;

  if (debug) { _tprintf(_T("Process: |"PS"|"PS"|\n"), fdir, fname); fflush(NULL); }

  if (subdirs) {
    if (_tcslen(fdir) > 0) {
      xstrncpy(xname, fdir, MAXP);
      xstrncat(xname, _T("*"), MAXP);
    }
    else xstrncpy(xname, _T("*"), MAXP);

    if (debug) { _tprintf(_T("Searching for dir |"PS"|\n"), xname); fflush(NULL); }

    ff = FindFirstFile(xname, &fa);
    if (ff == INVALID_HANDLE_VALUE) {
      error = GetLastError();
      if (error != ERROR_FILE_NOT_FOUND) {
        _ftprintf(stderr, _T("FindFirstFile: "PS" (%u)\n"), xstrerror(error), error);
        final_rc++;
        return;
      }
    }

    while (ff != INVALID_HANDLE_VALUE) {
      if ((fa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
          (fa.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0 &&
          _tcscmp(fa.cFileName, _T(".")) != 0 && _tcscmp(fa.cFileName, _T("..")) != 0) {

        if (debug) { _tprintf(_T("Found dir: |"PS"|\n"), fa.cFileName); fflush(NULL); }

        if (_tcslen(fdir) > 0) {
          xstrncpy(xdir, fdir, MAXP);
          xstrncat(xdir, fa.cFileName, MAXP);
          xstrncat(xdir, DIRSES, MAXP);

          xstrncpy(xname, fdir, MAXP);
          xstrncat(xname, fa.cFileName, MAXP);
        }
        else {
          xstrncpy(xdir, fa.cFileName, MAXP);
          xstrncat(xdir, DIRSES, MAXP);

          xstrncpy(xname, fa.cFileName, MAXP);
        }

        process(xdir, fname);

        // delete dir xname - not when we are deleting specific files recursively!
        //if (debug) { _tprintf(_T("Deleting dir: |"PS"|\n"), xname); fflush(NULL); }
        //rc = ShellDelete(xname, 1);
      } // if dir

      rc = FindNextFile(ff, &fa);
      if (rc == 0) {
        error = GetLastError();
        if (error != ERROR_NO_MORE_FILES) {
          _ftprintf(stderr, _T("FindNextFile: "PS" (%u)\n"), xstrerror(error), error);
          final_rc++;
          return;
        }
        FindClose(ff);
        ff = INVALID_HANDLE_VALUE;
      }
    } // for each file in dir
  } // recurse into subdirs

  if (_tcslen(fdir) > 0) {
    xstrncpy(xname, fdir, MAXP);
    xstrncat(xname, fname, MAXP);
  }
  else xstrncpy(xname, fname, MAXP);

  if (debug) { _tprintf(_T("Searching for file |"PS"|\n"), xname); fflush(NULL); }

  len = (int)_tcslen(xname);
  if (len == 1) 
    ask = (xname[len-1] == _T('*'));
  else if (len > 1)
    ask = (xname[len-2] == DIRSEP) && (xname[len-1] == _T('*'));
  else ask = 0;  // (s = _tcschr(xname, _T('*'))) != NULL
  if (ask && !quiet && !prompt)
    conf = getconf(xname, _T("Are you sure"));
  else conf = 1;

  ff = FindFirstFile(xname, &fa);
  if (ff == INVALID_HANDLE_VALUE) {
    error = GetLastError();
    if (error != ERROR_FILE_NOT_FOUND) {
      _ftprintf(stderr, _T("FindFirstFile: "PS" (%u)\n"), xstrerror(error), error);
      final_rc++;
      return;
    }
  }

  while (ff != INVALID_HANDLE_VALUE) {
    if ((fa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

      if (debug) { _tprintf(_T("Found file: |"PS"|\n"), fa.cFileName); fflush(NULL); }

      if (_tcslen(fdir) > 0) {
        xstrncpy(xname, fdir, MAXP);
        xstrncat(xname, fa.cFileName, MAXP);
      }
      else xstrncpy(xname, fa.cFileName, MAXP);

      fa_attrs = fa.dwFileAttributes & FILE_ATTRIBUTE_MASK;

      ro_fail = 0;
      if (pos_attrs) {  // all must be set
        flags = 0;
        if (fa_attrs & FILE_ATTRIBUTE_READONLY) {
          if (pos_attrs & FILE_ATTRIBUTE_READONLY) {
            flags |= FILE_ATTRIBUTE_READONLY;
            ro_doit = 1;
          }
          else ro_doit = force;
        }
        else if (pos_attrs & FILE_ATTRIBUTE_READONLY) ro_doit = 0;
        else ro_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_HIDDEN) {
          if (pos_attrs & FILE_ATTRIBUTE_HIDDEN) {
            flags |= FILE_ATTRIBUTE_HIDDEN;
            hid_doit = 1;
          }
          else hid_doit = 0;
        }
        else if (pos_attrs & FILE_ATTRIBUTE_HIDDEN) hid_doit = 0;
        else hid_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_SYSTEM) {
          if (pos_attrs & FILE_ATTRIBUTE_SYSTEM) {
            flags |= FILE_ATTRIBUTE_SYSTEM;
            sys_doit = 1;
          }
          else sys_doit = 0;
        }
        else if (pos_attrs & FILE_ATTRIBUTE_SYSTEM) sys_doit = 0;
        else sys_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_ARCHIVE) {
          if (pos_attrs & FILE_ATTRIBUTE_ARCHIVE)
            flags |= FILE_ATTRIBUTE_ARCHIVE;
          ar_doit = 1;
        }
        else if (pos_attrs & FILE_ATTRIBUTE_ARCHIVE) ar_doit = 0;
        else ar_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) {
          if (pos_attrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
            flags |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
          nci_doit = 1;
        }
        else if (pos_attrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) nci_doit = 0;
        else nci_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_REPARSE_POINT) {
          if (pos_attrs & FILE_ATTRIBUTE_REPARSE_POINT)
            flags |= FILE_ATTRIBUTE_REPARSE_POINT;
          rp_doit = 1;
        }
        else if (pos_attrs & FILE_ATTRIBUTE_REPARSE_POINT) rp_doit = 0;
        else rp_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_OFFLINE) {
          if (pos_attrs & FILE_ATTRIBUTE_OFFLINE)
            flags |= FILE_ATTRIBUTE_OFFLINE;
          off_doit = 1;
        }
        else if (pos_attrs & FILE_ATTRIBUTE_OFFLINE) off_doit = 0;
        else off_doit = 1;

        if (neg_attrs) {  // any can be set
          if (flags == pos_attrs) {
            pos_doit = ro_doit;
            ro_fail = !pos_doit;
          }
          else pos_doit = ro_doit && hid_doit && sys_doit &&
                          ar_doit && nci_doit && rp_doit && off_doit;
          neg_doit = fa_attrs & neg_attrs;
        }
        else {
          if (flags == pos_attrs) {
            pos_doit = ro_doit;
            ro_fail = !pos_doit;
          }
          else pos_doit = ro_doit && hid_doit && sys_doit &&
                          ar_doit && nci_doit && rp_doit && off_doit;
          neg_doit = 0;
        }
      }
      else {
        if (fa_attrs & FILE_ATTRIBUTE_READONLY) { ro_doit = force; ro_fail = !ro_doit; }
        else ro_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_HIDDEN) hid_doit = 0;
        else hid_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_SYSTEM) sys_doit = 0;
        else sys_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_ARCHIVE) ar_doit = 1;
        else ar_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) nci_doit = 1;
        else nci_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_REPARSE_POINT) rp_doit = 1;
        else rp_doit = 1;
        if (fa_attrs & FILE_ATTRIBUTE_OFFLINE) off_doit = 1;
        else off_doit = 1;

        if (neg_attrs) {  // any can be set
          neg_doit = fa_attrs & neg_attrs;
          pos_doit = !neg_doit;
        }
        else {
          // protected files
          pos_doit = ro_doit && hid_doit && sys_doit;
          neg_doit = 0;
        }
      }
      //1 h-r: zbrisi 3(h), 23 ... 32(h...),
      //2 ha-s: zbrisi 25(ha), 28(hia), can't: 15(rha), 18(rhia)
      //3 rho-s-i: zbrisi 16(ha)
      //_ftprintf(stderr, _T(""PS" - patt: %d, natt: %d - ro: %d (fail: %d), hid: %d, sys: %d - pos: %d, neg: %d\n"),
      //          fa.cFileName, pos_attrs, neg_attrs, ro_doit, ro_fail, hid_doit, sys_doit, pos_doit, neg_doit);

      doit = pos_doit && !neg_doit;

      if (prompt)
        conf = getconf(xname, _T("Delete"));

      if (doit) {
        files_found++;
        if (conf) {
          // delete file xname
          if (debug) { _tprintf(_T("Deleting file: |"PS"|\n"), xname); fflush(NULL); }
          rc = ShellDelete(xname, subdirs);
        }
      }
      else if (ro_fail && hid_doit && sys_doit && !neg_doit) {
        files_found++;
        if (conf)
          _ftprintf(stderr, _T("Access denied - "PS"\n"), xname);
      }
    } // if file

    rc = FindNextFile(ff, &fa);
    if (rc == 0) {
      error = GetLastError();
      if (error != ERROR_NO_MORE_FILES) {
        _ftprintf(stderr, _T("FindNextFile: "PS" (%u)\n"), xstrerror(error), error);
        final_rc++;
        return;
      }
      FindClose(ff);
      ff = INVALID_HANDLE_VALUE;
    }
  } /* for each file in dir */

  if (ff != INVALID_HANDLE_VALUE) FindClose(ff);
  SetLastError(error);

  fflush(NULL);
} /* process */


// ----------------------------------------------------------------------------
// parse_attrs
// parse attributes from command line switch /A
// ----------------------------------------------------------------------------
int parse_attrs(_TCHAR **sptr)
{
  _TCHAR *s = *sptr;
  int ii, neg;

  // start with /A...
  //             ^here
  ii = 0; neg = 0;
  while (*++s) {
    switch (_totlower(*s)) {
      case _T(':'):  // skip
        if (ii > 0) goto error_ret;  // allowed only at the beginning
        break;
      case _T('-'):  // negative
        if (neg) goto error_ret;
        neg = 1;
        break;
      case _T('r'):
        if (neg) neg_attrs |= FILE_ATTRIBUTE_READONLY;
        else pos_attrs |= FILE_ATTRIBUTE_READONLY;
        neg = 0;
        break;
      case _T('h'):
        if (neg) neg_attrs |= FILE_ATTRIBUTE_HIDDEN;
        else pos_attrs |= FILE_ATTRIBUTE_HIDDEN;
        neg = 0;
        break;
      case _T('i'):
        if (neg) neg_attrs |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
        else pos_attrs |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
        neg = 0;
        break;
      case _T('s'):
        if (neg) neg_attrs |= FILE_ATTRIBUTE_SYSTEM;
        else pos_attrs |= FILE_ATTRIBUTE_SYSTEM;
        neg = 0;
        break;
      case _T('a'):
        if (neg) neg_attrs |= FILE_ATTRIBUTE_ARCHIVE;
        else pos_attrs |= FILE_ATTRIBUTE_ARCHIVE;
        neg = 0;
        break;
      case _T('l'):
        if (neg) neg_attrs |= FILE_ATTRIBUTE_REPARSE_POINT;
        else pos_attrs |= FILE_ATTRIBUTE_REPARSE_POINT;
        neg = 0;
        break;
      case _T('o'):
        if (neg) neg_attrs |= FILE_ATTRIBUTE_OFFLINE;
        else pos_attrs |= FILE_ATTRIBUTE_OFFLINE;
        neg = 0;
        break;
      default:
        goto error_ret;
    } // switch
    ii++;
  } // while
  *sptr = --s;  // return with ptr to last character
  if (neg) {
error_ret:
    _ftprintf(stderr, _T("Parameter format not correct - \""PS"\"\n"), s);
    return 1;
  }
  return 0;
} /* parse_attrs */


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
  _tprintf(_T("Deletes one or more files.\n\n"));
  _tprintf(_T(""PS" [/P] [/F] [/S] [/Q] [/A[[:]attributes]] [/D] [/V] names\n\n"), prog);
  _tprintf(_T("  names         Specifies a list of one or more files or directories.\n"));
  _tprintf(_T("                Wildcards may be used to delete multiple files. If a\n"));
  _tprintf(_T("                directory is specified, all files within the directory\n"));
  _tprintf(_T("                will be deleted.\n\n"));
  _tprintf(_T("  /P            Prompts for confirmation before deleting each file.\n"));
  _tprintf(_T("  /F            Force deleting of read-only files.\n"));
  _tprintf(_T("  /S            Delete specified files from all subdirectories.\n"));
  _tprintf(_T("  /Q            Quiet mode, do not ask if ok to delete on global wildcard\n"));
  _tprintf(_T("  /A            Selects files to delete based on attributes\n"));
  _tprintf(_T("  attributes    R  Read-only files            S  System files\n"));
  _tprintf(_T("                H  Hidden files               A  Files ready for archiving\n"));
  _tprintf(_T("                I  Not content indexed Files  L  Reparse Points\n"));
  _tprintf(_T("                O  Offline files              -  Prefix meaning not\n"));
  _tprintf(_T("  /D            Enables additional debug output.\n"));
  _tprintf(_T("  /V            Prints program version and exits.\n\n"));
  _tprintf(_T("The display semantics of the /S switch are reversed in that it shows\n"));
  _tprintf(_T("you only the files that are deleted, not the ones it could not find.\n"));
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
  int ii, ac, opt, len, rc;
  unsigned int error;
  _TCHAR *s, *av[MAXC];
  _TCHAR fpname[MAXP+1], *fnp;
  _TCHAR fdir[MAXP+1], fname[MAXP+1];
  struct _stat64 sb;

  // get program name
  if ((prog = _tcsrchr(argv[0], DIRSEP)) == NULL) prog = argv[0];
  else prog++;
  if ((s = xstrcasestr(prog, _T(".exe"))) != NULL) *s = NULLC;
  for (ii = 0; ii < (int)_tcslen(prog); ii++)
    prog[ii] = _totupper(prog[ii]);

  // default global flags
  debug = 0;  // no debug
  prompt = 0;
  force = 0;
  subdirs = 0;
  quiet = 0;
  pos_attrs = 0;
  neg_attrs = 0;

  // parse command line
  ac = 0; opt = 1;
  for (ii = 1; ii < argc && ac < MAXC; ii++) {
    if (opt && *argv[ii] == _T('/')) {
      s = argv[ii];
      while (*++s) {
        switch (_totlower(*s)) {
          case _T('/'):  // skip
            break;
          case _T('p'):  // prompt
            prompt = 1;
            break;
          case _T('f'):  // force
            force = 1;
            break;
          case _T('s'):  // subdirs
            subdirs = 1;
            break;
          case _T('q'):  // quiet
            quiet = 1;
            break;
          case _T('a'):  // attributes
            if (parse_attrs(&s)) exit(1);
            break;
          case _T('d'):  // debug
            debug++;
            break;
          case _T('v'):  // version
            print_version();
            exit(0);
            break;
          case _T('?'):  // help
error:      print_version();
            print_usage();
            exit(2);
          default:
            _ftprintf(stderr, _T("Invalid switch - \""PS"\"\n"), s);
            exit(1);
        } // switch
      } // while
      continue;
    } // if (opt && /)
    av[ac] = (_TCHAR *)malloc((MAXL+1) * sizeof(_TCHAR));
    if (av[ac] == NULL) {
      error = GetLastError();
      _ftprintf(stderr, _T("malloc: "PS" (%u)\n"), xstrerror(error), error);
      exit(11);
    }
    if (_tcslen(argv[ii]) > 0)
      xstrncpy(av[ac++], argv[ii], MAXL);
    else
      xstrncpy(av[ac++], _T("*"), MAXL);
  } // for (argc)
  av[ac] = NULL;

  if (ac == 0) goto error;
//  _ftprintf(stderr, _T("The syntax of the command is incorrect.\n"));
//  exit(1);
//}

  final_rc = 0;
  for (ii = 0; ii < ac; ii++) {
    //if (debug) { _tprintf(_T("Parameter %d: |"PS"|\n"), ii, av[ii]); fflush(NULL); }
    rc = GetFullPathName(av[ii], MAXP, (LPTSTR)&fpname, &fnp);
    if (rc == 0) {
      error = GetLastError();
      _ftprintf(stderr, _T(""PS": "PS" (%u)\n"), av[ii], xstrerror(error), error);
      final_rc++;
      continue;
    }
    //if (debug) { _tprintf(_T("Full path name: |"PS"|, fnp: |"PS"|\n"), fpname, fnp); fflush(NULL); }

    // check if specified parameter is actually a directory
    rc = _tstat64(fpname, &sb);
    if (rc == 0 && sb.st_mode & _S_IFDIR) {
      //if (debug) { _tprintf(_T("directory\n")); fflush(NULL); }
      xstrncpy(fdir, fpname, MAXP);
      len = (int)_tcslen(fpname);
      if (fpname[len-1] != DIRSEP)
        xstrncat(fdir, DIRSES, MAXP);
      xstrncpy(fname, _T("*"), MAXP);
    }
    else { // also rc < 0
      //if (debug) { _tprintf(_T("file\n")); fflush(NULL); }
      if (fnp == NULL) xstrncpy(fname, _T("*"), MAXP);  // probably a directory
      else { xstrncpy(fname, fnp, MAXP); *fnp = NULLC; }
      xstrncpy(fdir, fpname, MAXP);
      len = (int)_tcslen(fpname);
      if (fpname[len-1] != DIRSEP)
        xstrncat(fdir, DIRSES, MAXP);
    }

    files_found = 0;
    process(fdir, fname);

    if (files_found == 0) {
      xstrncpy(fpname, fdir, MAXP);
      xstrncat(fpname, fname, MAXP);
      _ftprintf(stderr, _T("Could Not Find "PS"\n"), fpname);
    }
  } // for (ac)

  return final_rc;
} /* main */
