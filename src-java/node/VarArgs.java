package node;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.lang.reflect.Array;
import java.util.Arrays;

public class VarArgs {
     public static Object[] getVarArgs(Method method, Object[] args) {
        if (method.isVarArgs()) {
            Class<?>[] methodParameterTypes = method.getParameterTypes();
            return getVarArgs(args, methodParameterTypes);
        }
        return args;
     }

     public static Object[] getVarArgs(Constructor constructor, Object[] args) {
        if (constructor.isVarArgs()) {
            Class<?>[] constructorParameterTypes = constructor.getParameterTypes();
            return getVarArgs(args, constructorParameterTypes);
        }
        return args;
     }

     public static Object[] getVarArgs(Object[] args, Class<?>[] methodParameterTypes) {
	if(args.length == methodParameterTypes.length
                && args[args.length - 1].getClass().equals(methodParameterTypes[methodParameterTypes.length - 1])) {
            return args;
        }

        Object[] newArgs = new Object[methodParameterTypes.length];
        System.arraycopy(args, 0, newArgs, 0, methodParameterTypes.length - 1);
        Class<?> varArgComponentType = methodParameterTypes[methodParameterTypes.length - 1].getComponentType();
        int varArgLength = args.length - methodParameterTypes.length + 1;
        Object[] varArgsArray = (Object[])Array.newInstance(varArgComponentType, varArgLength);
//         System.out.println("varArgComponentType: " + varArgComponentType);
//         System.out.println("varArgsArray: " + Arrays.asList(varArgsArray).toString());
//         System.out.println("args: " + Arrays.asList(args).toString());
        System.arraycopy(args, methodParameterTypes.length - 1, varArgsArray, 0, varArgLength);
        newArgs[methodParameterTypes.length - 1] = varArgsArray;
        return newArgs;
    }
}
