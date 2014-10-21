class TestLambda
{
    public TestLambda() {}

    interface IntegerMath {
        int op(int a, int b);
    }

    public int testLambdaAddition(Integer x, Integer y) {
        IntegerMath addition = (a, b) -> a + b;
        return addition.op(x, y);
    }

    public int testLambdaSubtraction(Integer x, Integer y) {
        IntegerMath subtraction = (a, b) -> a - b;
        return subtraction.op(x, y);
    }
}
