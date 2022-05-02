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
