# DEL command replacement to Recycle Bin

Program DELETE in this repository can be used as an (almost) 1:1 replacement for
the Microsoft's cmd.exe internal DEL command with the following difference
- it will **delete files to Recycle Bin** instead of permanently deleting them

Deleted files can be later recovered from Recycle Bin.

_**Disclaimer:** Wildcard usage, prompts and attributes' selection follow original
DEL command as closely as possible, but there could be differences in use cases,
which were not tested._

## Usage

```
DELETE 1.11  Copyright (c) 2022 Matjaz Rihtar  (Aug 25, 2022)
Deletes one or more files.

DELETE [/P] [/F] [/S] [/Q] [/A[[:]attributes]] [/D] [/V] names

  names         Specifies a list of one or more files or directories.
                Wildcards may be used to delete multiple files. If a
                directory is specified, all files within the directory
                will be deleted.

  /P            Prompts for confirmation before deleting each file.
  /F            Force deleting of read-only files.
  /S            Delete specified files from all subdirectories.
  /Q            Quiet mode, do not ask if ok to delete on global wildcard
  /A            Selects files to delete based on attributes
  attributes    R  Read-only files            S  System files
                H  Hidden files               A  Files ready for archiving
                I  Not content indexed Files  L  Reparse Points
                O  Offline files              -  Prefix meaning not
  /D            Enables additional debug output.
  /V            Prints program version and exits.

The display semantics of the /S switch are reversed in that it shows
you only the files that are deleted, not the ones it could not find.
```

## Cmd.exe DEL command replacement

To replace Microsoft's DEL command, copy delete.exe to some directory
(preferably C:\Windows), create DOSKEY file with macro redefinition
of internal DEL command and set it to load at the start of cmd.exe.

**File macros.doskey**
```
del=C:\Windows\delete.exe $*
```

**File doskey.reg**

System wide:
```
[HKEY_LOCAL_MACHINE\Software\Microsoft\Command Processor]
"AutoRun"="@doskey /macrofile=\"C:\\Windows\\macros.doskey\""
```
User specific:
```
[HKEY_CURRENT_USER\Software\Microsoft\Command Processor]
"AutoRun"="@doskey /macrofile=\"C:\\Windows\\macros.doskey\""
```

After new cmd window is created, delete.exe will be used instead of internal
DEL command and files will be deleted to Recycle bin.

**Beware:** this macro redefinition doesn't work in batch files, where you have
to call delete.exe directly.

## How to compile

### Microsoft C (Visual Studio 2019)

```
cl /O2 /Wall delete.c shell32.lib user32.lib
```

### MinGW (MSYS, gcc 12)

```
gcc -O2 -Wall delete.c -o delete.exe -s
```

Program should compile without any warnings in both cases.

### Tests

Simple tests of functionality can be done with the supplied script `test.bat`
(edit where necessary, see beginning of script). For tests if files exist or
were successfully deleted an additional program `check.c` is supplied, which
needs to be compiled before the `test.bat` is executed.

## License

DELETE program is licensed under the MIT License.
  
Copyright &copy; 2022 Matjaz Rihtar
