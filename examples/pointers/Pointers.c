int swap(int* a, int* b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
    return 0;
}

int main()
{
    int a = 1;
    int b = 0;
    swap(&a, &b);
    return a;
}
