int sum(int xs[], int len)
{
    int sum = 0;
    for (int i = 0; i < len; i = i + 1) {
        sum = sum + xs[i];
    }
    return sum;
}

int sumMatr(int xs[][2], int rows)
{
    int sum = 0;
    for (int r = 0; r < rows; r = r + 1) {
        for (int c = 0; c < 2; c = c + 1) {
            sum = sum + xs[r][c];
        }
    }
    return sum;
}

int main()
{
    int xs[3];
    {
        xs[0] = 0;
        xs[1] = 1;
        xs[2] = 2;
    }

    int m[2][2];
    {
        m[0][0] = 0;
        m[0][1] = 1;
        m[1][0] = 2;
        m[1][1] = 3;
    }

    int allSum = sum(xs, 3) + sumMatr(m, 2);

    return allSum - 9;
}