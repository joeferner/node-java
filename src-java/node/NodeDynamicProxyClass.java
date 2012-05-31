package node;

public class NodeDynamicProxyClass implements java.lang.reflect.InvocationHandler
{
  private native Object callJs(int ptr, java.lang.reflect.Method m, Object[] args) throws Throwable;
  public int ptr;

  static {
    try{
      Runtime.getRuntime().load("/home/jshimty/dev/node-java/build/Release/nodejavabridge_bindings.node");
    }catch(Exception e){
      System.out.println(e.toString());
    }
  }

  public NodeDynamicProxyClass(int ptr) {
    this.ptr = ptr;
  }

  public Object invoke(Object proxy, java.lang.reflect.Method m, Object[] args) throws Throwable
  {
    return callJs(this.ptr, m, args);
  }
}
