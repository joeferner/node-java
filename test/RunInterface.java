
public class RunInterface {
  public static interface Interface0Arg {
    public void run();
  }

  public static interface Interface1Arg {
    public void run(String arg1);
  }

  public static interface InterfaceWithReturn {
    public int run(int arg1);
  }

  public void run0Args(Interface0Arg r) {
    r.run();
    r.run();
  }

  public void run1Args(Interface1Arg r) {
    r.run("test1");
    r.run("test1");
  }

  public int runWithReturn(InterfaceWithReturn r) {
    return r.run(42);
  }
}
