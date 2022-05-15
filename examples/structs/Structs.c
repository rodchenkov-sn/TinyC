struct Vec2d {
    int x[10];
};

struct Vec2d foo(struct Vec2d x)
{
    return x;
}

int main()
{
    struct Vec2d x;
    struct Vec2d y = foo(x);
    return 0;
}
