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
    Object result = callJs(this.ptr, m, args);
    //if(result == null) {
    //  System.out.println("invoke: null");
    //} else {
    //  System.out.println("invoke: " + result + " class: " + result.getClass() + " to string: " + result.toString());
    //}
    return result;
  }

  public void ref() {

  }

  public void unref() {

  }
}
