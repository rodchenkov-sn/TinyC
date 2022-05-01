int sum(int a, int b)
{
    int sum;
    {
        sum = a + b;
    }
    return sum;
}

int main()
{
    int a = 5;
    int b = 8;
    int s = sum(a, sum(10, b));
    return s;
}
