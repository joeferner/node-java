var pregyp = require('node-pre-gyp');

console.log('Setting environment path for JVM DLL');
process.env.PATH += require('./build/jvm_dll_path.json');

var pregypRun = new pregyp.Run();
pregypRun.parseArgv([ '--fallback-to-build' ]);

pregypRun.commands.install(pregypRun, function() {
    console.log('Done installing with node-pre-gyp');
});