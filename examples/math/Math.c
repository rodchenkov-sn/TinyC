int sum(int a, int b)
{
    int sum;
    {
        sum = a + b;
    }
    return sum;
}

int equals(int a, int b)
{
    return a == b;
}

int main()
{
    int a = 5;
    int b = 8;
    int t = equals(a, b);
    int s = sum(t, sum(1 >= 0, b));
    return s;
}
