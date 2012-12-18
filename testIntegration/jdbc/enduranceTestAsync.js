'use strict';

// Tests concurrent async jdbc connections.
// TODO: a proper jdbc wrapper should be created to avoid ugly async code.

var memwatch = require('memwatch');
var async = require('async');

// 1 - 23,305ms
// 5 - 9,338ms
// 10 - 8,846ms
var concurrency = 5;

//var dbServerName = '192.168.13.190';
//var dbPort = 1433;
//var dbName = 'test';
//var dbUserId = 'test';
//var dbPassword = 'test';
//var dbConnectString = 'jdbc:sqlserver://' + dbServerName + ':' + dbPort + ';databaseName=' + dbName + ';selectMethod=direct;responseBuffering=adaptive;packetSize=0;programName=nodeJavaTest;hostProcess=nodeJavaTest;sendStringParametersAsUnicode=false;';
//var dbConnectionClass = 'com.microsoft.sqlserver.jdbc.SQLServerDriver';

var dbUserId = 'test';
var dbPassword = 'test';
var dbConnectString = "jdbc:mysql://localhost/test";
var dbConnectionClass = 'com.mysql.jdbc.Driver';

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

  java.findClassSync(dbConnectionClass);

  var loopIterations = [];
  for (var i = 0; i < 5000; i++) {
    loopIterations.push(i);
  }

  async.forEachLimit(
    loopIterations,
    concurrency,
    function(loopCount, callback) {
      console.log('loopCount:', loopCount);
      return doLoop(callback);
    },
    function(err) {
      if (err) {
        console.log("fail", err);
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
    }
  );
}, 1);

function doLoop(callback) {
  var conn;

  return DriverManager.getConnection(dbConnectString, dbUserId, dbPassword, getConnectionComplete);

  function getConnectionComplete(err, _conn) {
    if (err) {
      return callback(err);
    }
    conn = _conn;

    //console.log("connected");
    var queryString = "select * from Person";
    return executeQuery(conn, queryString, function(row, callback) {
      //console.log("row", row);
      return callback();
    }, function(err) {
      if (err) {
        return callback(err);
      }
      //console.log("query complete");
      return callback();
    });
  }
}

function executeQuery(conn, sql, rowCallback, completeCallback) {
  var statement = conn.createStatementSync();
  return statement.executeQuery(sql, function(err, rs) {
    if (err) {
      return completeCallback(err);
    }

    var columnCount = rs.getMetaDataSync().getColumnCountSync();

    var rsComplete = false;
    async.until(
      function() { return rsComplete; },
      function(callback) {
        return rs.next(function(err, rsNextResult) {
          if (err) {
            return callback(err);
          }
          if (!rsNextResult) {
            rsComplete = true;
            return callback();
          }
          var row = [];
          for (var i = 1; i <= columnCount; i++) {
            row.push(rs.getObjectSync(i));
          }
          return rowCallback(row, callback);
        });
      },
      function(err) {
        if (err) {
          return completeCallback(err);
        }
        return conn.close(completeCallback);
      }
    );
  });
}