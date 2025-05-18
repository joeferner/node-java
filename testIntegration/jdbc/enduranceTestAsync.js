"use strict";

// Tests concurrent async jdbc connections.
// TODO: a proper jdbc wrapper should be created to avoid ugly async code.

const memwatch = require("memwatch");
const async = require("async");

// 1 - 23,305ms
// 5 - 9,338ms
// 10 - 8,846ms
const concurrency = 5;

//const dbServerName = '192.168.13.190';
//const dbPort = 1433;
//const dbName = 'test';
//const dbUserId = 'test';
//const dbPassword = 'test';
//const dbConnectString = 'jdbc:sqlserver://' + dbServerName + ':' + dbPort + ';databaseName=' + dbName + ';selectMethod=direct;responseBuffering=adaptive;packetSize=0;programName=nodeJavaTest;hostProcess=nodeJavaTest;sendStringParametersAsUnicode=false;';
//const dbConnectionClass = 'com.microsoft.sqlserver.jdbc.SQLServerDriver';

const dbUserId = "test";
const dbPassword = "test";
const dbConnectString = "jdbc:mysql://localhost/test";
const dbConnectionClass = "com.mysql.jdbc.Driver";

const util = require("util");
const path = require("path");
const java = require("../../");
java.classpath.push(path.join(__dirname, "sqljdbc4.jar"));
java.classpath.push(path.join(__dirname, "mysql-connector-java-5.1.22-bin.jar"));
const DriverManager = java.import("java.sql.DriverManager");

setTimeout(function () {
  console.log("start heap diff");
  const hd = new memwatch.HeapDiff();
  const loopStart = new Date();

  java.findClassSync(dbConnectionClass);

  const loopIterations = [];
  for (let i = 0; i < 5000; i++) {
    loopIterations.push(i);
  }

  async.forEachLimit(
    loopIterations,
    concurrency,
    function (loopCount, callback) {
      console.log("loopCount:", loopCount);
      return doLoop(callback);
    },
    function (err) {
      if (err) {
        console.log("fail", err);
      }
      const loopEnd = new Date();
      console.log("end loop", loopEnd - loopStart);
      memwatch.gc();
      const diff = hd.end();
      console.log(util.inspect(diff.change, false, 10, true));

      console.log("done... waiting 30seconds");
      setTimeout(function () {
        console.log("really done");
      }, 30 * 1000);
    }
  );
}, 1);

function doLoop(callback) {
  let conn;

  return DriverManager.getConnection(dbConnectString, dbUserId, dbPassword, getConnectionComplete);

  function getConnectionComplete(err, _conn) {
    if (err) {
      return callback(err);
    }
    conn = _conn;

    //console.log("connected");
    const queryString = "select * from Person";
    return executeQuery(
      conn,
      queryString,
      function (row, callback) {
        //console.log("row", row);
        return callback();
      },
      function (err) {
        if (err) {
          return callback(err);
        }
        //console.log("query complete");
        return callback();
      }
    );
  }
}

function executeQuery(conn, sql, rowCallback, completeCallback) {
  const statement = conn.createStatementSync();
  return statement.executeQuery(sql, function (err, rs) {
    if (err) {
      return completeCallback(err);
    }

    const columnCount = rs.getMetaDataSync().getColumnCountSync();

    let rsComplete = false;
    async.until(
      function () {
        return rsComplete;
      },
      function (callback) {
        return rs.next(function (err, rsNextResult) {
          if (err) {
            return callback(err);
          }
          if (!rsNextResult) {
            rsComplete = true;
            return callback();
          }
          const row = [];
          for (let i = 1; i <= columnCount; i++) {
            row.push(rs.getObjectSync(i));
          }
          return rowCallback(row, callback);
        });
      },
      function (err) {
        if (err) {
          return completeCallback(err);
        }
        return conn.close(completeCallback);
      }
    );
  });
}
