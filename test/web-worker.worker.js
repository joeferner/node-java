import { parentPort } from 'node:worker_threads';
import { getJava } from "../testHelpers.js";

parentPort.on('message', (message) => {
    console.log('message.name', message.name);
    switch (message.name) {
        case 'getJavaVersion':
            getJavaVersion();
            break;
        default:
            console.error('message', message);
            parentPort.postMessage(`unhandled name: ${message.name}`);
    }
});

function getJavaVersion() {
    console.log('a');
    const version = getJava().callStaticMethodSync("java.lang.System", "getProperty", "java.version");
    parentPort.postMessage(version);
}
