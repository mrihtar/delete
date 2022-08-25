@echo off
setlocal
@rem set DELCMD=del
set DELCMD=delete

call :create_all
if errorlevel 1 goto exit_error

@rem goto exit_ok

call :test_normal
if errorlevel 1 goto exit_error

call :test_attrib
if errorlevel 1 goto exit_error

:exit_ok
set RC=0
:final
endlocal
exit /b %RC%

:exit_error
set RC=1
echo [101mTest failed[m
goto final

@rem --------------------------------------------------------------------------
:create_all
check -q -ae x
if not errorlevel 1 (
  echo [33m---------------------------------------- Prompt 1x[m
  rmdir x /s
)
call :create_x
if errorlevel 1 exit /b 1

check -q -ae x\x1
if not errorlevel 1 (
  echo [33m---------------------------------------- Prompt 1x[m
  rmdir x\x1 /s
)
call :create_x1
if errorlevel 1 exit /b 1

check -q -ae x\x2
if not errorlevel 1 (
  echo [33m---------------------------------------- Prompt 1x[m
  rmdir x\x2 /s
)
call :create_x2
if errorlevel 1 exit /b 1

exit /b 0

@rem --------------------------------------------------------------------------
:create_x
check -q -ae x
if errorlevel 1 mkdir x
for /l %%G in (1,1,5) do echo This is %%G> x\c.%%G
attrib -a x\*

attrib +r x\c.2

exit /b 0

@rem --------------------------------------------------------------------------
:create_x1
check -q -ae x\x1
if errorlevel 1 mkdir x\x1
for /l %%G in (1,1,42) do echo This is %%G> x\x1\d.%%G
attrib -a x\x1\*

attrib +r x\x1\d.2
attrib +h x\x1\d.3
attrib +i x\x1\d.4
attrib +s x\x1\d.5
attrib +a x\x1\d.6
attrib +o x\x1\d.7

attrib +r +h x\x1\d.8
attrib +r +i x\x1\d.9
attrib +r +s x\x1\d.10
attrib +r +a x\x1\d.11
attrib +r +o x\x1\d.12

attrib +r +h +i x\x1\d.13
attrib +r +h +s x\x1\d.14
attrib +r +h +a x\x1\d.15
attrib +r +h +o x\x1\d.16

attrib +r +h +i +s x\x1\d.17
attrib +r +h +i +a x\x1\d.18
attrib +r +h +i +o x\x1\d.19

attrib +r +h +i +s +a x\x1\d.20
attrib +r +h +i +s +o x\x1\d.21

attrib +r +h +i +s +a +o x\x1\d.22

attrib +h +i x\x1\d.23
attrib +h +s x\x1\d.24
attrib +h +a x\x1\d.25
attrib +h +o x\x1\d.26

attrib +h +i +s x\x1\d.27
attrib +h +i +a x\x1\d.28
attrib +h +i +o x\x1\d.29

attrib +h +i +s +a x\x1\d.30
attrib +h +i +s +o x\x1\d.31

attrib +h +i +s +a +o x\x1\d.32

attrib +i +s x\x1\d.33
attrib +i +a x\x1\d.34
attrib +i +o x\x1\d.35

attrib +i +s +a x\x1\d.36
attrib +i +s +o x\x1\d.37

attrib +i +s +a +o x\x1\d.38

attrib +s +a x\x1\d.39
attrib +s +o x\x1\d.40

attrib +s +a +o x\x1\d.41

attrib +a +o x\x1\d.42

exit /b 0

@rem --------------------------------------------------------------------------
:create_x2
check -q -ae x\x2
if errorlevel 1 mkdir x\x2
for /l %%G in (1,1,4) do echo This is %%G> x\x2\e.%%G
attrib -a x\x2\*

%DELCMD% x\x2\e.2 /q
@rem e.2 <file junction --> e.1
mklink /j x\x2\e.2 x\x2\e.1 > NUL
%DELCMD% x\x2\e.4 /q
@rem e.4 <file symlink --> e.3
mklink x\x2\e.4 x\x2\e.3 > NUL

@rem x3 <dir junction --> x2
mklink /j x\x3 x\x2 > NUL
@rem x4 <dir symlink --> x2
mklink x\x4 x2 > NUL

exit /b 0

@rem --------------------------------------------------------------------------
:test_normal
echo [92m+=============================+[m
echo [92m^| Normal                      ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\c.1

check -q -an x\c.1
if errorlevel 1 ( echo [91mError 1[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Readonly no                 ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\c.2

check -q -ae x\c.2
if errorlevel 1 ( echo [91mError 2[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Wildcard 1                  ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1

echo [33m---------------------------------------- Prompt 1x[m
%DELCMD% x\*

check -q -an x\c.1 x\c.3 x\c.4 x\c.5
if errorlevel 1 ( echo [91mError 3[m & exit /b 1 )
check -q -ae x\c.2
if errorlevel 1 ( echo [91mError 3[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Wildcard 2                  ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1

echo [33m---------------------------------------- Prompt 1x[m
%DELCMD% x

check -q -an x\c.1 x\c.3 x\c.4 x\c.5
if errorlevel 1 ( echo [91mError 4[m & exit /b 1 )
check -q -ae x\c.2
if errorlevel 1 ( echo [91mError 4[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Wildcard 3                  ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\*.2 /s

check -q -ae x\c.2 x\x1\d.2 x\x2\e.2
if errorlevel 1 ( echo [91mError 5[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Wildcard prompt             ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1

echo [33m---------------------------------------- Prompt 5x[m
%DELCMD% x /p

check -q -an x\c.1 x\c.3 x\c.4 x\c.5
if errorlevel 1 ( echo [91mError 6[m & exit /b 1 )
check -q -ae x\c.2
if errorlevel 1 ( echo [91mError 6[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Readonly force              ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\c.2 /f

check -q -an x\c.2
if errorlevel 1 ( echo [91mError 7[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Follow junction             ^|[m
echo [92m+=============================+[m
call :create_x2 > NUL 2>&1
if errorlevel 1 exit /b 1

echo [33m---------------------------------------- Prompt 1x[m
%DELCMD% x\x3

check -q -an x\x2\e.1 x\x2\e.3 x\x2\e.4
if errorlevel 1 ( echo [91mError 8[m & exit /b 1 )
check -q -ae x\x2\e.2
if errorlevel 1 ( echo [91mError 8[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Subdirs 1                   ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1
call :create_x2 > NUL 2>&1
if errorlevel 1 exit /b 1

echo [33m---------------------------------------- Prompt 3x[m
%DELCMD% x\* /s

check -q -an x\c.1 x\c.3 x\c.4 x\c.5
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\c.2
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )

check -q -an x\x1\d.1 x\x1\d.4 x\x1\d.6 x\x1\d.7 x\x1\d.34 x\x1\d.35 x\x1\d.42
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x1\d.2 x\x1\d.3 x\x1\d.5 x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x1\d.21 x\x1\d.22 x\x1\d.23 x\x1\d.24 x\x1\d.25
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x1\d.26 x\x1\d.27 x\x1\d.28 x\x1\d.29 x\x1\d.30
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x1\d.31 x\x1\d.32 x\x1\d.33 x\x1\d.36 x\x1\d.37
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x1\d.38 x\x1\d.39 x\x1\d.40 x\x1\d.41
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )

check -q -an x\x2\e.1 x\x2\e.3 x\x2\e.4
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )
check -q -ae x\x2\e.2
if errorlevel 1 ( echo [91mError 9[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Subdirs 2                   ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1cd \util
call :create_x2 > NUL 2>&1
if errorlevel 1 exit /b 1

echo [33m---------------------------------------- Prompt 3x[m
%DELCMD% x /s

check -q -an x\c.1 x\c.3 x\c.4 x\c.5
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\c.2
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )

check -q -an x\x1\d.1 x\x1\d.4 x\x1\d.6 x\x1\d.7 x\x1\d.34 x\x1\d.35 x\x1\d.42
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x1\d.2 x\x1\d.3 x\x1\d.5 x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x1\d.21 x\x1\d.22 x\x1\d.23 x\x1\d.24 x\x1\d.25
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x1\d.26 x\x1\d.27 x\x1\d.28 x\x1\d.29 x\x1\d.30
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x1\d.31 x\x1\d.32 x\x1\d.33 x\x1\d.36 x\x1\d.37
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x1\d.38 x\x1\d.39 x\x1\d.40 x\x1\d.41
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )

check -q -an x\x2\e.1 x\x2\e.3 x\x2\e.4
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )
check -q -ae x\x2\e.2
if errorlevel 1 ( echo [91mError 10[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Subdirs quiet               ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1
call :create_x2 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x /s /q

check -q -an x\c.1 x\c.3 x\c.4 x\c.5
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\c.2
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )

check -q -an x\x1\d.1 x\x1\d.4 x\x1\d.6 x\x1\d.7 x\x1\d.34 x\x1\d.35 x\x1\d.42
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x1\d.2 x\x1\d.3 x\x1\d.5 x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x1\d.21 x\x1\d.22 x\x1\d.23 x\x1\d.24 x\x1\d.25
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x1\d.26 x\x1\d.27 x\x1\d.28 x\x1\d.29 x\x1\d.30
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x1\d.31 x\x1\d.32 x\x1\d.33 x\x1\d.36 x\x1\d.37
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x1\d.38 x\x1\d.39 x\x1\d.40 x\x1\d.41
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )

check -q -an x\x2\e.1 x\x2\e.3 x\x2\e.4
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )
check -q -ae x\x2\e.2
if errorlevel 1 ( echo [91mError 11[m & exit /b 1 )

exit /b 0

@rem --------------------------------------------------------------------------
:test_attrib
echo [92m+=============================+[m
echo [92m^| Single attributes           ^|[m
echo [92m+=============================+[m
call :create_x > NUL 2>&1
if errorlevel 1 exit /b 1
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1
call :create_x2 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.1
%DELCMD% x\x1\d.2 /a:r
%DELCMD% x\x1\d.3 /a:h
%DELCMD% x\x1\d.4 /a:i
%DELCMD% x\x1\d.5 /a:s
%DELCMD% x\x1\d.6 /a:a
%DELCMD% x\x1\d.7 /a:o

check -q -an x\x1\d.1 x\x1\d.2 x\x1\d.3 x\x1\d.4 x\x1\d.5 x\x1\d.6 x\x1\d.7
if errorlevel 1 ( echo [91mError 21[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Combination attributes 1    ^|[m
echo [92m+=============================+[m

%DELCMD% x\x1\d.8  /a:rh
%DELCMD% x\x1\d.9  /a:ri
%DELCMD% x\x1\d.10 /a:rs
%DELCMD% x\x1\d.11 /a:ra
%DELCMD% x\x1\d.12 /a:ro

%DELCMD% x\x1\d.13 /a:rhi
%DELCMD% x\x1\d.14 /a:rhs
%DELCMD% x\x1\d.15 /a:rha
%DELCMD% x\x1\d.16 /a:rho

%DELCMD% x\x1\d.17 /a:rhis
%DELCMD% x\x1\d.18 /a:rhia
%DELCMD% x\x1\d.19 /a:rhio

%DELCMD% x\x1\d.20 /a:rhisa
%DELCMD% x\x1\d.21 /a:rhiso

%DELCMD% x\x1\d.22 /a:rhisao

%DELCMD% x\x1\d.23 /a:hi
%DELCMD% x\x1\d.24 /a:hs
%DELCMD% x\x1\d.25 /a:ha
%DELCMD% x\x1\d.26 /a:ho

%DELCMD% x\x1\d.27 /a:his
%DELCMD% x\x1\d.28 /a:hia
%DELCMD% x\x1\d.29 /a:hio

%DELCMD% x\x1\d.30 /a:hisa
%DELCMD% x\x1\d.31 /a:hiso

%DELCMD% x\x1\d.32 /a:hisao

%DELCMD% x\x1\d.33 /a:is
%DELCMD% x\x1\d.34 /a:ia
%DELCMD% x\x1\d.35 /a:io

%DELCMD% x\x1\d.36 /a:isa
%DELCMD% x\x1\d.37 /a:iso

%DELCMD% x\x1\d.38 /a:isao

%DELCMD% x\x1\d.39 /a:sa
%DELCMD% x\x1\d.40 /a:so

%DELCMD% x\x1\d.41 /a:sao

%DELCMD% x\x1\d.42 /a:ao

check -q -an x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )
check -q -an x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )
check -q -an x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )
check -q -an x\x1\d.21 x\x1\d.22 x\x1\d.23 x\x1\d.24 x\x1\d.25
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )
check -q -an x\x1\d.26 x\x1\d.27 x\x1\d.28 x\x1\d.29 x\x1\d.30
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )
check -q -an x\x1\d.31 x\x1\d.32 x\x1\d.33 x\x1\d.34 x\x1\d.35
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )
check -q -an x\x1\d.36 x\x1\d.37 x\x1\d.38 x\x1\d.39 x\x1\d.40
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )
check -q -an x\x1\d.41 x\x1\d.42
if errorlevel 1 ( echo [91mError 22[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Combination attributes 2    ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:r /q

check -q -an x\x1\d.2 x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 23[m & exit /b 1 )
check -q -an x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 23[m & exit /b 1 )
check -q -an x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 23[m & exit /b 1 )
check -q -an x\x1\d.21 x\x1\d.22
if errorlevel 1 ( echo [91mError 23[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "26" ( echo [91mError 23[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Combination attributes 3    ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:rh /q

check -q -an x\x1\d.8 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 24[m & exit /b 1 )
check -q -an x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 24[m & exit /b 1 )
check -q -an x\x1\d.21 x\x1\d.22
if errorlevel 1 ( echo [91mError 24[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "31" ( echo [91mError 24[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Combination attributes 4    ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:rhs /q

check -q -an x\x1\d.14 x\x1\d.17 x\x1\d.20 x\x1\d.21 x\x1\d.22
if errorlevel 1 ( echo [91mError 25[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "37" ( echo [91mError 25[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Negative attributes 1       ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:-r /q

check -q -ae x\x1\d.2 x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 26[m & exit /b 1 )
check -q -ae x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 26[m & exit /b 1 )
check -q -ae x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 26[m & exit /b 1 )
check -q -ae x\x1\d.21 x\x1\d.22
if errorlevel 1 ( echo [91mError 26[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "16" ( echo [91mError 26[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Negative attributes 2       ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:-r-h /q

check -q -ae x\x1\d.2 x\x1\d.3 x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 27[m & exit /b 1 )
check -q -ae x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 27[m & exit /b 1 )
check -q -ae x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 27[m & exit /b 1 )
check -q -ae x\x1\d.21 x\x1\d.22 x\x1\d.23 x\x1\d.24 x\x1\d.25
if errorlevel 1 ( echo [91mError 27[m & exit /b 1 )
check -q -ae x\x1\d.26 x\x1\d.27 x\x1\d.28 x\x1\d.29 x\x1\d.30
if errorlevel 1 ( echo [91mError 27[m & exit /b 1 )
check -q -ae x\x1\d.31 x\x1\d.32
if errorlevel 1 ( echo [91mError 27[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "27" ( echo [91mError 27[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Neg. ^& pos. attribs 1       ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:h-r /q

check -q -ae x\x1\d.1 x\x1\d.2 x\x1\d.4 x\x1\d.5
if errorlevel 1 ( echo [91mError 28[m & exit /b 1 )
check -q -ae x\x1\d.6 x\x1\d.7 x\x1\d.8 x\x1\d.9 x\x1\d.10
if errorlevel 1 ( echo [91mError 28[m & exit /b 1 )
check -q -ae x\x1\d.11 x\x1\d.12 x\x1\d.13 x\x1\d.14 x\x1\d.15
if errorlevel 1 ( echo [91mError 28[m & exit /b 1 )
check -q -ae x\x1\d.16 x\x1\d.17 x\x1\d.18 x\x1\d.19 x\x1\d.20
if errorlevel 1 ( echo [91mError 28[m & exit /b 1 )
check -q -ae x\x1\d.21 x\x1\d.22 x\x1\d.33 x\x1\d.34 x\x1\d.35
if errorlevel 1 ( echo [91mError 28[m & exit /b 1 )
check -q -ae x\x1\d.36 x\x1\d.37 x\x1\d.38 x\x1\d.39 x\x1\d.40
if errorlevel 1 ( echo [91mError 28[m & exit /b 1 )
check -q -ae x\x1\d.41 x\x1\d.42
if errorlevel 1 ( echo [91mError 28[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "31" ( echo [91mError 28[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Neg. ^& pos. attribs 2       ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:ha-s /q

@rem check -q -an x\x1\d.15 x\x1\d.18
@rem if errorlevel 1 ( echo [91mError 29[m & exit /b 1 )
check -q -an x\x1\d.25 x\x1\d.28
if errorlevel 1 ( echo [91mError 29[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "40" ( echo [91mError 29[m & exit /b 1 )

echo [92m+=============================+[m
echo [92m^| Neg. ^& pos. attribs 3       ^|[m
echo [92m+=============================+[m
call :create_x1 > NUL 2>&1
if errorlevel 1 exit /b 1

%DELCMD% x\x1\d.* /a:rho-s-i /q

check -q -an x\x1\d.16
if errorlevel 1 ( echo [91mError 30[m & exit /b 1 )

dir x\x1 /b /a > %TEMP%\count
for /F "tokens=3" %%G in ('find /v /c "" %TEMP%\count') do set NF=%%G
if "%NF%" NEQ "41" ( echo [91mError 30[m & exit /b 1 )

exit /b 0
