namespace geom {

struct Point2D {
    Point2D() = default;
    Point2D(double x, double y) {
        this->x = x;
        this->y = y;
    }

    double x = 0.0;
    double y = 0.0;
};

}