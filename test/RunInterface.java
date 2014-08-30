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
    return r.run(42);
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
}
