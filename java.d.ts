/* eslint-disable @typescript-eslint/no-unsafe-function-type */
/* eslint-disable @typescript-eslint/no-explicit-any */
// eslint-disable-next-line no-var
declare var NodeJavaCore: NodeJavaCore.NodeAPI;
export = NodeJavaCore;

declare namespace NodeJavaCore {
  type JavaObject = any;
  type JavaError = Error & { cause?: JavaObject };

  export interface Callback<T> {
    (err?: Error, result?: T): void;
  }

  export interface JavaCallback<T> {
    (err?: JavaError, result?: T): void;
  }

  interface Promisify {
    (fn: Function, receiver?: any): Function;
  }

  interface AsyncOptions {
    /**
     * Suffix for synchronous method call signatures.
     */
    syncSuffix: string;
    
    /**
     * Suffix for callback-based async method call signatures.
     */
    asyncSuffix?: string | undefined;

    /**
     * Suffix for promise-based async method call signatures
     */
    promiseSuffix?: string | undefined;

    /**
     * Callback-to-promise transform implementation. From Node.js version 8 one can
     * just use Node.js implementation: `promisify: require('util').promisify`.
     */
    promisify?: Promisify | undefined;

    /**
     * The JavaScript object returned by `java.import(classname)` is a JavaScript constructor
     * Function, implemented such that you can create instances of the Java class. For example:
     */
    ifReadOnlySuffix?: string | undefined;
  }

  interface ProxyFunctions {
    [index: string]: Function;
  }

  interface Java {
    /**
     * Array of paths or jars to pass to the creation of the JVM.
     * 
     * All items must be added to the classpath before calling any other node-java methods.
     * 
     * @example
     * java.classpath.push('commons.io.jar');
     * java.classpath.push('src');
     */
    classpath: string[];

    /**
     * Array of options to pass to the creation of the JVM.
     * 
     * All items must be added to the options before calling any other node-java methods.
     * 
     * @example
     * java.options.push('-Djava.awt.headless=true');
     * java.options.push('-Xmx1024m');
     */
    options: string[];

    /**
     * @see AsyncOptions
     */
    asyncOptions: AsyncOptions;

    /**
     * Location of nodejavabridge_bindings.node
     */
    nativeBindingLocation: string;

    /**
     * Calls a method on the specified instance. If you are using the sync method an exception
     * will be throw if an error occurs, otherwise it will be the first argument in the callback.
     *
     * @param instance An instance of the class from newInstance.
     * @param methodName The name of the method to call. The method name can include the full
     *                   signature (see [Getting the full method signature](README.md#getFullMethodSignature)).
     * @param args The arguments to pass to the method, the last argument will be the callback to the function
     *
     * @example
     * const instance = java.newInstanceSync("com.nearinfinty.MyClass");
     * java.callMethod(instance, "doSomething", 42, "test", function(err, results) {
     *   if(err) { console.error(err); return; }
     *   // results from doSomething
     * });
     */
    callMethod(instance: JavaObject, methodName: string, ...args: any[]): void;

    /**
     * Calls a method on the specified instance. If you are using the sync method an exception
     * will be throw if an error occurs, otherwise it will be the first argument in the callback.
     *
     * @param instance An instance of the class from newInstance.
     * @param methodName The name of the method to call. The method name can include the full
     *                   signature (see [Getting the full method signature](README.md#getFullMethodSignature)).
     * @param args The arguments to pass to the method
     * @returns The result of the method call
     *
     * @example
     * const instance = java.newInstanceSync("com.nearinfinty.MyClass");
     * const result = java.callMethodSync("com.nearinfinty.MyClass", "doSomething", 42, "test");
     */
    callMethodSync(instance: JavaObject, methodName: string, ...args: any[]): any;

    /**
     * Calls a static method on the specified class. If you are using the sync method an exception will be
     * throw if an error occurs, otherwise it will be the first argument in the callback.
     *
     * @param className The name of the class to call the method on. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param methodName The name of the method to call. The method name can include the full
     *                   signature (see [Getting the full method signature](README.md#getFullMethodSignature)).
     * @param args The arguments to pass to the method, the last argument will be the callback to the function
     */
    callStaticMethod(className: string, methodName: string, ...args: any[]): void;

    /**
     * Calls a static method on the specified class. If you are using the sync method an exception will be
     * throw if an error occurs, otherwise it will be the first argument in the callback.
     *
     * @param className The name of the class to call the method on. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param methodName The name of the method to call. The method name can include the full
     *                   signature (see [Getting the full method signature](README.md#getFullMethodSignature)).
     * @param args The arguments to pass to the method
     * @returns The result of the method call
     */
    callStaticMethodSync(className: string, methodName: string, ...args: any[]): any;

    /**
     * Finds the class with the specified binary name. This method should be overridden by class loader
     * implementations that follow the delegation model for loading classes, and will be invoked by the
     * loadClass method after checking the parent class loader for the requested class. The default
     * implementation throws a ClassNotFoundException.
     *
     * @param className The binary name of the class
     * @returns The resulting Class object
     */
    findClassSync(className: string): JavaObject;

    /**
     * Gets a static field value from the specified class.
     *
     * @param className The name of the class to get the value from. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param fieldName The name of the field to get the value from.
     * @returns The valid of the static field
     */
    getStaticFieldValue(className: string, fieldName: string): any;

    /**
     * Sets a static field value on the specified class.
     *
     * @param className The name of the class to set the value on. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param fieldName The name of the field to set the value on.
     * @param newValue The new value to assign to the field.
     */
    setStaticFieldValue(className: string, fieldName: string, newValue: any): void;

    /**
     * Determines of a javaObject is an instance of a class.
     *
     * @param javaObject Instance of a java object returned from a method or from newInstance.
     * @param className A string class name.
     *
     * @example
     * const obj = java.newInstanceSync("my.package.SubClass");
     * if(java.instanceOf(obj, "my.package.SuperClass")) {
     *   console.log("obj is an instance of SuperClass");
     * }
     */
    instanceOf(javaObject: JavaObject, className: string): boolean;

    /**
     * Register that a client wants to be called back immediately before and/or immediately
     * after the JVM is created. If used, this function must be called before the JVM has been
     * created. The before function is typically used to add to the classpath. The function may
     * execute asynchronous operations (such as a async glob function). The after function is
     * sometimes useful for doing one-time initialization that requires the JVM to first be
     * initialized. If either function is unnecessary, use `null` or `undefined`. See also
     * `registerClientP` and `ensureJvm`. See the unit tests in `testAsyncOptions` for examples.
     */
    registerClient(
      before: ((cb: Callback<void>) => void) | undefined | null,
      after?: (cb: Callback<void>) => void
    ): void;

    /**
     * Register that a client wants to be called back immediately before and/or immediately
     * after the JVM is created. If used, this function must be called before the JVM has been
     * created. The before function is typically used to add to the classpath. The function may
     * execute asynchronous operations (such as a async glob function). The after function is
     * sometimes useful for doing one-time initialization that requires the JVM to first be
     * initialized. If either function is unnecessary, use `null` or `undefined`. See also
     * `registerClientP` and `ensureJvm`. See the unit tests in `testAsyncOptions` for examples.
     */
    registerClientP(beforeP: (() => Promise<void>) | undefined | null, afterP?: () => Promise<void>): void;

    /**
     * If the JVM has not yet been created, execute the full JVM initialization process, then
     * call callback function when initialization is complete. If the JVM has been created, just
     * call the callback. Note that the full initialization process includes: 1) executing all
     * registered client *before* hooks, 2) creating the JVM, then 3) executing all registered
     * client *after* hooks.
     */
    ensureJvm(done: Callback<void>): void;

    /**
     * If the JVM has not yet been created, execute the full JVM initialization process, then
     * call callback function when initialization is complete. If the JVM has been created, just
     * call the callback. Note that the full initialization process includes: 1) executing all
     * registered client *before* hooks, 2) creating the JVM, then 3) executing all registered
     * client *after* hooks.
     */
    ensureJvm(): Promise<void>;

    /**
     * Returns true if the JVM has been created. The JVM can only be created once.
     */
    isJvmCreated(): boolean;

    /**
     * Creates a new java byte. This is needed because JavaScript does not have the concept of a byte.
     */
    newByte(val: number): JavaObject;

    /**
     * Creates a new java short. This is needed because JavaScript does not have the concept of a short.
     */
    newShort(val: number): JavaObject;

    /**
     * Creates a new java long. This is needed because JavaScript does not have the concept of a long.
     */
    newLong(val: number): JavaObject;

    /**
     * Creates a new java char. This is needed because JavaScript does not have the concept of a char.
     */
    newChar(val: string | number): JavaObject;

    /**
     * Creates a new java float. This is needed to force JavaScript's number to a float to call some methods.
     */
    newFloat(val: number): JavaObject;

    /**
     * Creates a new java double. This is needed to force JavaScript's number to a double to call some methods.
     */
    newDouble(val: number): JavaObject;

    /**
     * Loads the class given by className such that it acts and feels like a JavaScript object.
     *
     * @param className The name of the class to create. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     *
     * @example
     * const Test = java.import('Test');
     * Test.someStaticMethodSync(5);
     * console.log(Test.someStaticField);
     *
     * const value1 = Test.NestedEnum.Value1;
     *
     * const test = new Test();
     * list.instanceMethodSync('item1');
     */
    import(className: string): JavaObject;

    /**
     * Creates an instance of the specified class. If you are using the sync method an exception will
     * be throw if an error occurs, otherwise it will be the first argument in the callback.
     *
     * @param className The name of the class to create. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param args Arguments to pass to the constructor, the last argument is a callback function
     */
    newInstance(className: string, ...args: any[]): void;

    /**
     * Creates an instance of the specified class. If you are using the sync method an exception will
     * be throw if an error occurs, otherwise it will be the first argument in the callback.
     *
     * @param className The name of the class to create. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param args Arguments to pass to the constructor
     */
    newInstanceSync(className: string, ...args: any[]): JavaObject;

    /**
     * Creates a new java array of given glass type. To create array of primitive types
     * like `char`, `byte`, etc, pass the primitive typename
     * (eg. `java.newArray("char", "hello world\n".split(''))`).
     *
     * @param className The name of the type of array elements. Separate nested classes
     *                  using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param arg A JavaScript array of values to assign to the java array.
     */
    newArray(className: string, arg: any[]): JavaObject;

    /**
     * Get the current class loader
     */
    getClassLoader(): JavaObject;

    /**
     * Creates a new java Proxy for the given interface. Functions passed in will run on the v8
     * main thread and not a new thread.
     *
     * The returned object has a method unref() which you can use to free the object for garbage
     * collection.
     *
     * @param interfaceName The name of the interface to proxy. Separate nested classes
     *                      using `'$'` (eg. `com.nearinfinty.MyClass$NestedClass`).
     * @param functions A hash of functions matching the function in the interface.
     *
     * @example
     * const myProxy = java.newProxy('java.lang.Runnable', {
     *   run: function () {
     *     // This is actually run on the v8 thread and not the new java thread
     *     console.log("hello from thread");
     *   }
     * });
     *
     * const thread = java.newInstanceSync("java.lang.Thread", myProxy);
     * thread.start();
     */
    newProxy(interfaceName: string, functions: ProxyFunctions): JavaObject;

    /**
     * Stops the running event loop
     */
    stop(): void;
  }
}
