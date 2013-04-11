
public class Test {
  private int i;
  public int nonstaticInt = 42;
  public static int staticFieldInt = 42;
  public static double staticFieldDouble = 42.5;
  public static Test[] staticArrayObjects = null;

  public Test() {}
  public Test(int i) { this.i = i; }

  public int getInt() { return i; }

  public int methodOverload(String a) { return 1; }
  public int methodOverload(int a) { return 2; }
  public int methodOverload(SuperClass a) { return a.getVal(); }

  public static String staticMethod() { return "staticMethod called"; }
  public static int staticMethod(int i) { return i + 1; }
  public static void staticMethodThrows(Exception ex) throws Exception { throw ex; }
  public void methodThrows(Exception ex) throws Exception { throw ex; }

  public static int staticMethodOverload(String a) { return 1; }
  public static int staticMethodOverload(int a) { return 2; }
  public static int staticMethodOverload(SuperClass a) { return a.getVal(); }

  public static String staticMethodCharArrayToString(char[] a) { return new String(a); }
  public static String staticMethodLongToString(java.lang.Long l) { return l.toString(); }
  public static long staticMethodReturnLong() { return java.lang.Long.MAX_VALUE; }

  public static class SuperClass {
    public int getVal() { return 3; }
  }

  public static class SubClass extends SuperClass {
    public int getVal() { return 4; }
  }

  public static int[] getArrayOfInts() {
    int arr[] = new int[5];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;
    return arr;
  }

  public static byte[] getArrayOfBytes() {
    byte arr[] = new byte[5];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;
    return arr;
  }

  public static boolean[] getArrayOfBools() {
    boolean arr[] = new boolean[5];
    arr[0] = true;
    arr[1] = true;
    arr[2] = false;
    arr[3] = true;
    arr[4] = false;
    return arr;
  }

  public static double[] getArrayOfDoubles() {
    double arr[] = new double[5];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;
    return arr;
  }

  public static float[] getArrayOfFloats() {
    float arr[] = new float[5];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;
    return arr;
  }

  public static long[] getArrayOfLongs() {
    long arr[] = new long[5];
    arr[0] = Long.MAX_VALUE;
    arr[1] = Long.MIN_VALUE;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;
    return arr;
  }
}
