package node;

public class NodeDynamicProxyClass implements java.lang.reflect.InvocationHandler
{
  private native Object callJs(long ptr, java.lang.reflect.Method m, Object[] args) throws Throwable;
  public long ptr;

  public NodeDynamicProxyClass(String path, long ptr) {
    try{
      Runtime.getRuntime().load(path);
    }catch(Exception e){
      System.out.println(e.toString());
    }
    this.ptr = ptr;
  }

  public Object invoke(Object proxy, java.lang.reflect.Method m, Object[] args) throws Throwable
  {
    return callJs(this.ptr, m, args);
  }
}
