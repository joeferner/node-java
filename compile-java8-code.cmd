set JAVA_VERSION=1.8

set OUTFILE="%TMP%\findJavaHome.txt"
node findJavaHome.js > %OUTFILE%
set /p JAVA_HOME=<%OUTFILE%

cd test8
"%JAVA_HOME%\bin\javac" -source %JAVA_VERSION% *.java
