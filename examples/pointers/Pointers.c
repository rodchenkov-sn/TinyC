void swap(int* a, int* b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int main()
{
    int a = 1;
    int b = 0;
    swap(&a, &b);
    return a;
}
