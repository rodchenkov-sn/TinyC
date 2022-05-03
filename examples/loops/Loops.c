int factorial1(int x)
{
    if (x <= 0) {
        return 0;
    }
    int f = 1;
    while (x > 0) {
        f = f * x;
        x = x - 1;
    }
    return f;
}

int factorial2(int x)
{
    int f = 1;
    for (int i = 1; i <= x; i = i + 1) {
        f = f * i;
    }
    return f;
}

int main()
{
    return factorial1(5) + factorial2(5);
}
