struct Vec2d {
    int x;
    int y;
};

struct Vec2d add_by_val(struct Vec2d a, struct Vec2d b)
{
    struct Vec2d s;
    s.x = a.x + b.x;
    s.y = a.y + b.y;
    return s;
}

struct Vec2d add_by_ptr(struct Vec2d* a, struct Vec2d* b)
{
    struct Vec2d s;
    s.x = a->x + b->x;
    s.y = a->y + b->y;
    return s;
}

int len_sq(struct Vec2d* a)
{
    return a->x + a->y;
}

int main()
{
    struct Vec2d a;
    {
        a.x = 1;
        a.y = 2;
    }

    struct Vec2d b;
    {
        b.x = 3;
        b.y = 4;
    }

    struct Vec2d sum1 = add_by_val(a, a);
    struct Vec2d sum2 = add_by_ptr(&b, &b);

    int len1 = len_sq(&sum1);
    int len2 = len_sq(&sum2);

    int sum_len = len1 + len2;

    return sum_len - 20;
}
