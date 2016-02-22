
public class Test {
  private int i;
  public int nonstaticInt = 42;
  public static int staticFieldInt = 42;
  public static double staticFieldDouble = 42.5;
  public static Test[] staticArrayObjects = null;

  public Test() {}
  public Test(Integer i) { this.i = i; }
  public Test(Integer i, String... other) { this.i = i; }

  public int getInt() { return i; }

  public int methodOverload(String a) { return 1; }
  public int methodOverload(int a) { return 2; }
  public int methodOverload(SuperClass a) { return a.getVal(); }

  public static String staticMethod() { return "staticMethod called"; }
  public static int staticMethod(int i) { return i + 1; }
  public static void staticMethodThrows(Exception ex) throws Exception { throw ex; }
  public void methodThrows(Exception ex) throws Exception { throw ex; }
  public static void staticMethodThrowsNewException() throws Exception { throw new Exception("my exception"); }
  public void methodThrowsNewException() throws Exception { throw new Exception("my exception"); }

  public static int staticMethodOverload(String a) { return 1; }
  public static int staticMethodOverload(int a) { return 2; }
  public static int staticMethodOverload(SuperClass a) { return a.getVal(); }

  public static String staticMethodCharArrayToString(char[] a) { return new String(a); }
  public static String staticMethodLongToString(java.lang.Long l) { return l.toString(); }
  public static long staticMethodReturnLong() { return java.lang.Long.MAX_VALUE; }

  public static boolean static2Objects(Object o1, Object o2) { return o1.equals(o2); }

  public static int staticByte(byte b) { return (int)b; }
  public static int staticShort(short s) { return (int)s; }
  public static int staticLong(long l) { return (int)l; }
  public static double staticDouble(double s) { return s; }
  public static float staticFloat(float s) { return s; }
  public static String staticString(String s) { return s; }

  public static int staticMethodAmbiguous(Double a) { return 1; }
  public static int staticMethodAmbiguous(Integer a) { return 2; }

  public int methodAmbiguous(Double a) { return 1; }
  public int methodAmbiguous(Integer a) { return 2; }

  public static String staticVarargs(Integer i, String... args) {
    java.lang.StringBuilder result = new java.lang.StringBuilder();
    result.append(i);
    for(String arg : args) {
      result.append(arg);
    }
    return result.toString();
  }

  public static String staticBigDecimalToString(java.math.BigDecimal bigDecimal) { return bigDecimal.toString(); }

  public static int staticChar(char ch) { return (int)ch; }
  public static short[] staticShortArray(Short[] arg) {
    short[] b = new short[arg.length];
    for(int i=0; i<arg.length; i++) { b[i] = arg[i]; }
    return b;
  }
  public static short[] staticShortArray(short[] arg) {
    short[] b = new short[arg.length];
    for(int i=0; i<arg.length; i++) { b[i] = arg[i]; }
    return b;
  }
  public static boolean[] staticBooleanArray(boolean[] arg) {
    boolean[] b = new boolean[arg.length];
    for(int i=0; i<arg.length; i++) { b[i] = arg[i]; }
    return b;
  }
  public static boolean[] staticBooleanArray(Boolean[] arg) {
    boolean[] b = new boolean[arg.length];
    for(int i=0; i<arg.length; i++) { b[i] = arg[i]; }
    return b;
  }

  public static double[] staticDoubleArray(double[] arg) {
    double[] b = new double[arg.length];
    for(int i=0; i<arg.length; i++) { b[i] = arg[i]; }
    return b;
  }

  public static int[] staticIntArray(int[] arg) {
    int[] b = new int[arg.length];
    for(int i=0; i<arg.length; i++) { b[i] = arg[i]; }
    return b;
  }

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

  public static String getUnicodeBMP() {
    return new String(Character.toChars(0x2605));
  }

  public static String getUnicodeSMP() {
    return new String(Character.toChars(0x1F596));
  }

  public static String getUnicodeNull() {
    return new String("\0");
  }

  public static enum StaticEnum {
    Value1,
    Value2
  }
  public static String staticEnumToString(StaticEnum e) { return e.toString(); }

  public static String varArgsSignature(Object... args) { return "Object..."; }
  public static String varArgsSignature(Boolean... args) { return "Boolean..."; }
  public static String varArgsSignature(Double... args) { return "Double..."; }
  public static String varArgsSignature(Integer... args) { return "Integer..."; }
  public static String varArgsSignature(Long... args) { return "Long..."; }
  public static String varArgsSignature(Number... args) { return "Number..."; }
  public static String varArgsSignature(String... args) { return "String..."; }

  // The Javascript object returned by java.import(classname) is a Function object
  // so that it can be used as a constructor.
  // Javascript reserves some properties of Function as non-writable or non-configurable:
  //  'name', 'arguments', 'caller'.
  // This means we can't expose a static member such as 'name()' with that name.
  // We instead append a suffix (asyncOptions.ifReadOnlySuffix)
  // The following static members are used for unit tests involving these cases.

  // For testing static member functions with reserved names
  public static String name() { return "name"; }
  public static String arguments() { return "arguments"; }
  public static String caller() { return "caller"; }

  // For testing public static fields with reserved names
  public enum Enum {
    foo, bar,                 // non-reserved
    name, arguments, caller   // reserved
  };
}
