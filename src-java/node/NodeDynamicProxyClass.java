package node;

import java.lang.reflect.Method;

public class NodeDynamicProxyClass implements java.lang.reflect.InvocationHandler {
  private static final Method EQUALS;
  private static final Method HASHCODE;
  static {
    try {
      EQUALS = Object.class.getMethod("equals", Object.class);
      HASHCODE = Object.class.getMethod("hashCode");
    } catch (NoSuchMethodException e) {
      throw new ExceptionInInitializerError(e);
    }
  }

  private native Object callJs(long ptr, java.lang.reflect.Method m, Object[] args) throws Throwable;
  private native void unref(long ptr) throws Throwable;
  public final long ptr;

  public NodeDynamicProxyClass(String path, long ptr) {
    try{
      Runtime.getRuntime().load(path);
    }catch(Exception e){
      System.out.println(e.toString());
    }
    this.ptr = ptr;
  }

  @Override
  public Object invoke(Object proxy, java.lang.reflect.Method m, Object[] args) throws Throwable
  {
    try {
      Object result = callJs(this.ptr, m, args);
      //if(result == null) {
      //  System.out.println("invoke: null");
      //} else {
      //  System.out.println("invoke: " + result + " class: " + result.getClass() + " to string: " + result.toString());
      //}
      return result;
    } catch (NoSuchMethodError e) {
      // use 'vanilla' implementations otherwise - the object that persists between multiple invocations is
      // 'this', not the 'proxy' argument, so we operate on this.
      if (EQUALS.equals(m)) {
        // need to check if the arg is a Proxy, and if so, if its invocation handler == this!
        return args[0] == proxy;
      } else if (HASHCODE.equals(m)) {
        return System.identityHashCode(proxy);
      } else if ("unref".equals(m.getName()) && m.getParameterTypes().length == 0 && m.getReturnType() == Void.TYPE) {
        this.unref();
      }
      throw e;
    }
  }

  public void unref() throws Throwable {
    unref(this.ptr);
  }
}
