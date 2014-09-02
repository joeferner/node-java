import java.util.concurrent.CountDownLatch;

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
    try {
      return r.run(42);
    } catch (RuntimeException e) {
      throw new RuntimeException(e);
    }
  }

  public int runInAnotherThread(final InterfaceWithReturn r) throws InterruptedException {
    final int[] result = {0};
    final CountDownLatch latch = new CountDownLatch(1);
    Thread t = new Thread(new Runnable() {
      @Override
      public void run() {
        result[0] = r.run(46);
        latch.countDown();
      }
    });
    t.start();
    latch.await();
    return result[0];
  }

  public boolean runEquals(final InterfaceWithReturn r) {
    return r.equals(Boolean.FALSE);
  }

  public int runHashCode(final InterfaceWithReturn r) {
    return r.hashCode();
  }

  private InterfaceWithReturn prev;

  public void setInstance(final InterfaceWithReturn r) {
    prev = r;
  }

  public boolean runEqualsInstance(final InterfaceWithReturn r) {
    return r.equals(prev);
  }

  public String runToString(final InterfaceWithReturn r) {
    return r.toString();
  }
}
