set JAVA_VERSION=1.7

set OUTFILE="%TMP%\findJavaHome.txt"
node findJavaHome.js > %OUTFILE%
set /p JAVA_HOME=<%OUTFILE%

set JAVAC_OPTS=-source %JAVA_VERSION% -target %JAVA_VERSION% -bootclasspath "%JAVA_HOME%\jre\lib\rt.jar"

cd test
"%JAVA_HOME%\bin\javac" %JAVAC_OPTS% *.java

cd ..\src-java\node
"%JAVA_HOME%\bin\javac" %JAVAC_OPTS% *.java

cd ..\..\
"%JAVA_HOME%\bin\javah" -classpath src-java -d .\src node.NodeDynamicProxyClass