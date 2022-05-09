int sum(int xs[], int len)
{
    int sum = 0;
    for (int i = 0; i < len; i = i + 1) {
        sum = sum + xs[i];
    }
    return sum;
}

void add(int xs[], int ys[], int ss[], int len)
{
    for (int i = 0; i < len; i = i + 1) {
        ss[i] = xs[i] + ys[i];
    }
}

int main()
{
    int xs[3];
    { xs[0] = 0; xs[1] = 1; xs[2] = 2; }
    int ys[3];
    { ys[0] = 2; ys[1] = 1; ys[2] = 0; }
    int ss[3];
    add(xs, ys, ss, 3);
    return sum(ss, 3);
}