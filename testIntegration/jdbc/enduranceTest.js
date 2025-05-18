"use strict";

const memwatch = require("memwatch");

const dbServerName = "192.168.13.190";
const dbPort = 1433;
const dbName = "test";
const dbUserId = "test";
const dbPassword = "test";
const dbConnectString =
  "jdbc:sqlserver://" +
  dbServerName +
  ":" +
  dbPort +
  ";databaseName=" +
  dbName +
  ";selectMethod=direct;responseBuffering=adaptive;packetSize=0;programName=nodeJavaTest;hostProcess=nodeJavaTest;sendStringParametersAsUnicode=false;";
const dbConnectionClass = "com.microsoft.sqlserver.jdbc.SQLServerDriver";

//const dbUserId = 'test';
//const dbPassword = 'test';
//const dbConnectString = "jdbc:mysql://localhost/test";
//const dbConnectionClass = 'com.mysql.jdbc.Driver';

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
  for (let loopCount = 0; loopCount < 500000; loopCount++) {
    console.log("loopCount:", loopCount);
    doLoop();
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
}, 1);

function doLoop() {
  java.findClassSync(dbConnectionClass);
  const conn = DriverManager.getConnectionSync(dbConnectString, dbUserId, dbPassword);
  //console.log("connected");
  const statement = conn.createStatementSync();
  const queryString = "select * from Person";
  const rs = statement.executeQuerySync(queryString);
  const metaData = rs.getMetaDataSync();
  const columnCount = metaData.getColumnCountSync();
  while (rs.nextSync()) {
    for (let i = 1; i <= columnCount; i++) {
      const obj = rs.getObjectSync(i);
      if (obj) {
        if (Object.prototype.hasOwnProperty.call(obj, "getClassSync")) {
          if (obj.getClassSync().toString() == "class java.math.BigDecimal") {
            //console.log(obj.doubleValueSync());
            continue;
          }
          if (obj.getClassSync().toString() == "class java.sql.Timestamp") {
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
