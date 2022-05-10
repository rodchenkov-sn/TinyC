int max(int a, int b, int def)
{
    if (a > b) {
        return a;
    } else if (b > a) {
        return b;
    } else {
        return def;
    }
}

int main()
{
    int a = 5;
    int b = 3;
    b = max(a, b, 0);
    return max(a, b, 0);
}
