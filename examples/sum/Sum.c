int sum(int a, int b)
{
    return a + b;
    a = 1;
}

int main()
{
    int a = 5;
    int b = 8;
    int s = sum(a, sum(10, b));
    return s;
}
