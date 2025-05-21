const { parentPort } = require('node:worker_threads');
const path = require('node:path');

parentPort.on('message', (message) => {
    const requirePath = path.join(__dirname, '..');
    console.log('aaaaaaaaaaaaaaaaaaaaaaaaaa', requirePath);
    const java = require(requirePath);
    console.log('zzzzzzzzzzzzzzzzzzzzzzzzzz', java);

    console.log('message', message);
    parentPort.postMessage(42);
});
