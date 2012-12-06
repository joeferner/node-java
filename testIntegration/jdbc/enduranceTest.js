'use strict';

//var dbServerName = '';
//var dbPort = 1433;
//var dbName = '';
//var dbUserId = '';
//var dbPassword = '';
//var dbConnectString = 'jdbc:sqlserver://' + dbServerName + ':' + dbPort + ';databaseName=' + dbName + ';selectMethod=direct;responseBuffering=adaptive;packetSize=0;programName=nodeJavaTest;hostProcess=nodeJavaTest;sendStringParametersAsUnicode=false;';
//var dbConnectionClass = 'com.microsoft.sqlserver.jdbc.SQLServerDriver';

var dbUserId = 'test';
var dbPassword = 'test';
var dbConnectString = "jdbc:mysql://localhost/test";
var dbConnectionClass = 'com.mysql.jdbc.Driver';

var path = require('path');
var java = require('../../');
java.classpath.push(path.join(__dirname, 'sqljdbc4.jar'));
java.classpath.push(path.join(__dirname, 'mysql-connector-java-5.1.22-bin.jar'));
var DriverManager = java.import('java.sql.DriverManager');

for (var loopCount = 0; loopCount < 500000; loopCount++) {
  console.log('loopCount:', loopCount);
  java.findClassSync(dbConnectionClass);
  var conn = DriverManager.getConnectionSync(dbConnectString, dbUserId, dbPassword);
  console.log("connected");
  var statement = conn.createStatementSync();
  var queryString = "select * from Person";
  var rs = statement.executeQuerySync(queryString);
  var metaData = rs.getMetaDataSync();
  var columnCount = metaData.getColumnCountSync();
  while (rs.nextSync()) {
    for (var i = 1; i <= columnCount; i++) {
      console.log(rs.getObjectSync(i));
    }
  }
  conn.closeSync();
}

console.log("done... waiting 30seconds");
setTimeout(function() {
  console.log("really done");
}, 30 * 1000);