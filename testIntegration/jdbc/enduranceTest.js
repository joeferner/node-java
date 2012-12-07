'use strict';

var memwatch = require('memwatch');

var dbServerName = '192.168.13.190';
var dbPort = 1433;
var dbName = 'test';
var dbUserId = 'test';
var dbPassword = 'test';
var dbConnectString = 'jdbc:sqlserver://' + dbServerName + ':' + dbPort + ';databaseName=' + dbName + ';selectMethod=direct;responseBuffering=adaptive;packetSize=0;programName=nodeJavaTest;hostProcess=nodeJavaTest;sendStringParametersAsUnicode=false;';
var dbConnectionClass = 'com.microsoft.sqlserver.jdbc.SQLServerDriver';

//var dbUserId = 'test';
//var dbPassword = 'test';
//var dbConnectString = "jdbc:mysql://localhost/test";
//var dbConnectionClass = 'com.mysql.jdbc.Driver';

var util = require('util');
var path = require('path');
var java = require('../../');
java.classpath.push(path.join(__dirname, 'sqljdbc4.jar'));
java.classpath.push(path.join(__dirname, 'mysql-connector-java-5.1.22-bin.jar'));
var DriverManager = java.import('java.sql.DriverManager');

setTimeout(function() {
  console.log('start heap diff');
  var hd = new memwatch.HeapDiff();
  var loopStart = new Date();
  for (var loopCount = 0; loopCount < 500000; loopCount++) {
    console.log('loopCount:', loopCount);
    doLoop();
  }
  var loopEnd = new Date();
  console.log('end loop', loopEnd - loopStart);
  memwatch.gc();
  var diff = hd.end();
  console.log(util.inspect(diff.change, false, 10, true));

  console.log("done... waiting 30seconds");
  setTimeout(function() {
    console.log("really done");
  }, 30 * 1000);
}, 1);

function doLoop() {
  java.findClassSync(dbConnectionClass);
  var conn = DriverManager.getConnectionSync(dbConnectString, dbUserId, dbPassword);
  //console.log("connected");
  var statement = conn.createStatementSync();
  var queryString = "select * from Person";
  var rs = statement.executeQuerySync(queryString);
  var metaData = rs.getMetaDataSync();
  var columnCount = metaData.getColumnCountSync();
  while (rs.nextSync()) {
    for (var i = 1; i <= columnCount; i++) {
      var obj = rs.getObjectSync(i);
      if (obj) {
        if (obj.hasOwnProperty('getClassSync')) {
          if (obj.getClassSync().toString() == 'class java.math.BigDecimal') {
            //console.log(obj.doubleValueSync());
            continue;
          }
          if (obj.getClassSync().toString() == 'class java.sql.Timestamp') {
            //console.log(obj.getTimeSync());
            continue;
          }
          //console.log("class:", obj.getClassSync().toString());
        }
        //console.log(obj);
      }
    }
  }
  conn.closeSync();
}