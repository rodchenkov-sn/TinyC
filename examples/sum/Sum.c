int sum(bar a, int b)
{
    int s = a + b;
    return s;
}

int main()
{
    int a = 5;
    int b = 8;
    int s = sum(a, sum(10, b));
    return s;
}
