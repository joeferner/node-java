require('find-java-home')(function(err, home){
  var build;
  if(err){
    console.error("[node-java] "+err);
    process.exit(1);
  }
  process.stdout.write(home);
});
