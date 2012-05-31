package node;

public class NodeDynamicProxyClass implements java.lang.reflect.InvocationHandler
{
  private native Object callJs(int ptr, java.lang.reflect.Method m, Object[] args) throws Throwable;
  public int ptr;

  public NodeDynamicProxyClass(String path, int ptr) {
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
