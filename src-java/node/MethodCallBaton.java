package node;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

public abstract class MethodCallBaton {
    public static Object newInstance(Constructor<?> ctor, Object[] args) throws Exception {
        return ctor.newInstance(args);
    }

    public static Object invokeMethod(Method func, Object obj, Object[] args) throws Exception {
        func.setAccessible(true);
        return func.invoke(obj, args);
    }

    public static Object invokeMethod9(Method func, Object obj, Object[] args) throws Exception {
        if (!func.canAccess(obj)) {
            Method accessible = findAccessible(func, obj, func.getDeclaringClass());

            if (accessible != null) {
                func = accessible;
            }
        }

        return func.invoke(obj, args);
    }

    private static Method findAccessible(Method func, Object obj, Class<?> clazz) {
        Method accessible = null;

        if (clazz != null) {
            try {
                accessible = clazz.getMethod(func.getName(), func.getParameterTypes());
                accessible = accessible.canAccess(obj) ? accessible : null;
            } catch (Exception ignored) {
                accessible = null;
            }

            if (accessible == null) {
                accessible = findAccessible(func, obj, clazz.getSuperclass());
            }

            if (accessible == null) {
                for (Class<?> iface : clazz.getInterfaces()) {
                    accessible = findAccessible(func, obj, iface);

                    if (accessible != null) {
                        break;
                    }
                }
            }
        }

        return accessible;
    }
}
