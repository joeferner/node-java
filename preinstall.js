var FIND_JAVA_HOME_INSTALL_FAILED = 1;
var JAVA_HOME_NOT_FOUND = 2;
var BUILDING_NODE_JAVA_FAILED = 3;

var packageJSON = require('./package.json');

var fs = require('fs');
var path = require('path');
var spawn = require('child_process').spawn;
var stdio = [process.stdin, process.stdout, process.stderr];
var update;

removePreinstall();

log("Preparing to install find-java-home.");

update = spawn(
  'npm'
  , ['install', 'find-java-home']
  , {
    cwd:__dirname
    , stdio:stdio
  }
);

update.on('error', function(err){
  exit(err, FIND_JAVA_HOME_INSTALL_FAILED);
});

update.on('close', function(code){
  if(code)return exit(
    "Failed to install find-java-home. the given code was: "+code
    , FIND_JAVA_HOME_INSTALL_FAILED
  );


  log("Preparing to find JAVA_HOME");
  require('find-java-home')(function(err, home){
    var build;
    if(err)return exit(err, JAVA_HOME_NOT_FOUND);


    log("JAVA_HOME determined to be: "+home);
    log("Preparing to install node-java");
    build = spawn(
      'npm'
      , ['install']
      , {
        cwd:__dirname
        , stdio:stdio
        , 'env' :{
          HOME:process.env.HOME,
          JAVA_HOME:home,
          USERPROFILE:process.env.USERPROFILE
        }
      }
    );

    build.on('error', function(err){
      restorePreinstall();
      exit(err, BUILDING_NODE_JAVA_FAILED);
    });

    build.on('close', function(code){
      restorePreinstall();
      if(code)return exit(
        "Failed to build node-java. the given code was: "+code
        , BUILDING_NODE_JAVA_FAILED
      );
    });
  });
});

function error(msg){
  console.error("[node-java] "+msg);
}

function exit(msg, code){
  if(msg && code){
    error(msg);
  } else if(msg){
    log(msg);
  }
  process.exit(code || 0);
}

function log(msg){
  console.log("[node-java] "+msg);
}

function removePreinstall(){
  delete packageJSON.scripts.preinstall;
  fs.writeFileSync(
    path.resolve(__dirname, 'package.json')
    , JSON.stringify(packageJSON, null, "  ")
  );
}

function restorePreinstall(){
  packageJSON.scripts.preinstall = 'node preinstall.js';
  fs.writeFileSync(
    path.resolve(__dirname, 'package.json')
    , JSON.stringify(packageJSON, null, "  ")
  );
}

process.on("uncaughtException", function(){
  restorePreinstall();
});
