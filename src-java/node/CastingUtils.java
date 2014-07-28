package node;

import java.lang.reflect.Method;

public class CastingUtils {
    public static void cast(Method method, Object[] args) throws Throwable {
        Class[] methodParameterTypes = method.getParameterTypes();
        if (methodParameterTypes.length != args.length) {
            throw new Exception("Method argument length mismatch. Expecting " + methodParameterTypes.length + " found " + args.length);
        }
        for (int i = 0; i < methodParameterTypes.length; i++) {
            args[i] = cast(args[i], methodParameterTypes[0]);
        }
    }

    public static Object cast(Object o, Class t) {
        if (o == null) {
            return null;
        }

        Class oClass = o.getClass();
        if (oClass == Integer.class) {
            Integer i = (Integer) o;
            if (t == Double.class) {
                return i.doubleValue();
            }
        } else if (oClass == Double.class) {
            Double d = (Double) o;
            if (t == Integer.class) {
                return d.intValue();
            }
        }

        return o;
    }
}
