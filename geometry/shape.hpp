#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>

using std::vector;
using std::pair;
using std::abs;
using std::make_pair;
using std::min;
using std::max;

const double epsilon = 1e-3;

bool equal(double first, double second) {
  return abs(first - second) < epsilon;
}

double deg_to_rad(double angle) {
  return (angle / 180) * M_PI;
}

double rad_to_deg(double angle) {
  return (angle / M_PI) * 180;
}


class Line;

struct Point {
  double x;
  double y;

  Point() : x(0.), y(0.) {}

  Point(double x, double y) : x(x), y(y) {}

  Point(Point first, Point second) {
    x = first.x + second.x;
    x /= 2;
    y = first.y + second.y;
    y /= 2;
  }

  void rotate(const Point& center, double rad) {
    double x_2 = center.x + cos(rad) * (x - center.x) - sin(rad) * (y - center.y);
    double y_2 = center.y + sin(rad) * (x - center.x) + cos(rad) * (y - center.y);
    x = x_2;
    y = y_2;
  }

  void scale(const Point& center, double coe) {
    x = (x - center.x) * coe + center.x;
    y = (y - center.y) * coe + center.y;
  }

  void reflect(const Point& center) {
    scale(center, -1);
  }

  void reflect(const Line&);
};

double operator*(const Point first, const Point second) {
  return first.x * second.x + first.y * second.y;
}

Point operator+(const Point first, const Point second) {
  return {first.x + second.x, first.y + second.y};
}

Point operator-(const Point first, const Point second) {
  return {first.x - second.x, first.y - second.y};
}

bool operator==(const Point& first, const Point& second) {
  return equal(first.x, second.x) && equal(first.y, second.y);
}

bool operator!=(const Point& first, const Point& second) { return !(first == second); }

double dist(const Point& first, const Point& second) {
  return sqrt(pow((first.x - second.x), 2) + pow((first.y - second.y), 2));
}

double vecMultiply(const Point& first, const Point& second) {
  return first.x * second.y - first.y * second.x;
}

bool between(const Point& first, const Point& second, const Point target) {
  return (min(first.x, second.x) <= target.x) && (max(first.x, second.x) >= target.x) &&
         (min(first.y, second.y) <= target.y) && (max(first.y, second.y) >= target.y);
}


class Line {
 public:
  double a_;
  double b_;
  double c_;

 private:
  void normalise() {
    if (!equal(b_, 1.) && !equal(b_, 0.)) {
     a_ /= b_;
     c_ /= b_;
     b_ = 1.;
    }
  }

  Line(double a, double b, double c) : a_(a), b_(b), c_(c) {
    normalise();
  };

 public:
  Line(const Point& first, const Point& second) {
    a_ = first.y - second.y;
    b_ = second.x - first.x;
    c_ = first.x * second.y - second.x * first.y;
    normalise();
  }

  Line(double k, double b) {
    a_ = -k;
    b_ = 1;
    c_ = -b;
  }

  Line(const Point& point, double k) : a_(-k), b_(1) {
    c_ = point.x * k - point.y;
  }
};

bool operator==(const Line& first, const Line& second) {
  return equal(first.a_, second.a_) &&
  equal(first.b_, second.b_) &&
  equal(first.c_, second.c_);
}

bool operator!=(const Line& first, const Line& second) {
  return !(first == second);
}

Line orthogonal(const Line line, const Point axis = {0, 0}) {
  Point n_1 = Point(axis.x, axis.y);
  Point n_2 = Point(axis.x + line.a_, axis.y + line.b_);
  return {n_1, n_2};
}

int type(const Line& first, const Line& second) {
  if (equal(first.b_, 0)) {
    if (equal(second.b_, 0)) {
      if (equal(first.a_ / first.c_, second.a_ / second.c_)) {
        return 0;
      } else {
        return -1;
      }
    } else {
      return 1;
    }
  } else {
    if (equal(second.b_, 0)) {
      return 1;
    } else {
      if (equal(first.a_ / first.b_, second.a_ / second.b_)) {
        if (equal(first.c_ / first.b_, second.c_ / second.b_)) {
          return 0;
        } else {
          return -1;
        }
      }
    }
  }
  return 1;
}

Point intersect(const Line& first, const Line& second) {
  assert(!equal(first.a_ * second.b_ - second.a_ * first.b_, 0));
  double x = (first.b_ * second.c_ - second.b_ * first.c_) /
             (first.a_ * second.b_ - second.a_ * first.b_);
  double y = (first.c_ * second.a_ - second.c_ * first.a_) /
             (first.a_ * second.b_ - second.a_ * first.b_);
  return {x, y};
}


void Point::reflect(const Line& line) {
  Line ort = orthogonal(line, *this);
  Point inter = intersect(line, ort);
  reflect(inter);
}


class Shape {
 public:
  virtual double perimeter() = 0;

  virtual double area() const = 0;

  virtual bool operator==(const Shape& another) const = 0;

  bool operator!=(const Shape& another) const {
    return !(*this == another);
  }

  virtual bool isCongruentTo(const Shape& another) const = 0;

  virtual bool isSimilarTo(const Shape& another) const = 0;

  virtual bool containsPoint(const Point& point) const = 0;

  virtual void rotate(const Point& center, double angle) = 0;

  virtual void reflect(const Point& center) = 0;

  virtual void reflect(const Line& axis) = 0;

  virtual void scale(const Point& center, double coefficient) = 0;

  virtual ~Shape() = default;
};

class Polygon : public Shape {
 protected:
  vector<Point> vertices_;

  Point vert(int pos) const {
    int ver = verticesCount();
    return vertices_[((pos % ver) + ver) % ver];
  }

 public:
  void print() const {
    for (int i = 0; i < verticesCount(); ++i){
      std::cout << '{' << vert(i).x << ", " << vert(i).y << "}, ";
    }
    std::cout << '\n';
  }

  int verticesCount() const {
    return vertices_.size();
  }

  vector<Point> getVertices() const {
    vector ver = vertices_;
    return ver;
  }

  bool isConvex() const {
    bool flag = true;
    for (int i = 0; i < verticesCount(); ++i) {
      Point p_1 = vert(i) - vert(i + 1);
      Point p_2 = vert(i + 2) - vert(i + 1);
      if (vecMultiply(p_1, p_2) < 0) {
        flag = false;
        break;
      }
    }
    if (flag) {
      return true;
    }
    flag = true;
    for (int i = 0; i < verticesCount(); ++i) {
      Point p_1 = vert(i) - vert(i + 1);
      Point p_2 = vert(i + 2) - vert(i + 1);
      if (vecMultiply(p_2, p_1) < 0) {
        flag = false;
      }
    }
    if (flag) {
      return true;
    }
    return false;
  }

  Polygon(const vector<Point>& ver) : vertices_(ver) {}

  template<typename... T>
  Polygon(T... points) : vertices_({points...}) {};

  double perimeter() override {
    if (vertices_.size() <= 1) {
      return 0;
    }
    double su = 0;
    for (int i = 0; i < verticesCount(); ++i) {
      su += dist(vert(i - 1), vert(i));
    }
    return su;
  }

  double area() const override {
    double su = 0;
    int count = verticesCount();
    for (int i = 0; i < count; ++i) {
      su += vertices_[i].x * vertices_[(i + 1) % count].y -
            vertices_[i].y * vertices_[(i + 1) % count].x;
    }
    return fabsl(su / 2);
  }

  bool operator==(const Shape& another) const override {
    auto casted = dynamic_cast<const Polygon*>(&another);
    if (casted == nullptr || casted->verticesCount() != verticesCount()) {
      return false;
    } else {
      bool flag;
      int count = verticesCount();
      for (int k = -1; k < 2; k += 2) {
        for (int i = 0; i < count; ++i) {
          flag = true;
          for (int j = 0; j < count; ++j) {
            if (vert(j) != casted->vert((i + j) * k)) {
              flag = false;
              break;
            }
          }
          if (flag) {
            return true;
          }
        }
      }
      return false;
    }
  }

  bool isCongruentTo(const Shape& another) const override {
    bool f_1 = isSimilarTo(another);
    bool f_2 = equal(another.area(), area());
    bool flag = f_1 && f_2;
    return flag;
  }

  bool isSimilarTo(const Shape& another) const override {
    auto casted = dynamic_cast<const Polygon*>(&another);
    if (casted == nullptr || casted->verticesCount() != verticesCount()) {
      return false;
    }
    bool flag = false;
    int count = verticesCount();
    if (count == 1) {
      return true;
    }
    for (int k = 1; k > -2; k -= 2) {
      for (int i = 0; i < count; ++i) {
        flag = true;
        double coe = dist(casted->vert((i + 1) * k), casted->vert(i * k));
        coe /= dist(vert(1), vert(0));
        for (int j = 0; j < count; ++j) {
          Point v1 = vert(j) - vert(j + 1);
          Point v2 = vert(j + 2) - vert(j + 1);
          double v_cos = (v1 * v2) / sqrt(v1.x * v1.x + v1.y * v1.y) /
              sqrt(v2.x * v2.x + v2.y * v2.y);
          Point w1 = casted->vert((i + j) * k) - casted->vert((i + j + 1) * k);
          Point w2 = casted->vert((i + j + 2) * k) - casted->vert((i + j + 1) * k);
          double w_cos = (w1 * w2) / sqrt(w1.x * w1.x + w1.y * w1.y) /
              sqrt(w2.x * w2.x + w2.y * w2.y);
          if (!(equal(dist(casted->vert((i + j) * k),
                           casted->vert((i + j + 1) * k)),
                      coe * dist(vert(j), vert(j + 1))) &&
              equal(v_cos, w_cos))) {
            flag = false;
            break;
          }
        }
        if (flag) {
          return true;
        }
      }
    }
    return false;
  }

  bool containsPoint(const Point& point) const override {
    if (verticesCount() <= 2) {
      return false;
    }
    int count = verticesCount();
    int amount = 0;
    Line line = Line(point, point + Point(1, 1));
    for (int i = 0; i < count; ++i) {
      auto cur = vertices_[i];
      auto next = vert(i + 1);
      if (equal(cur.x * line.a_ + cur.y * line.b_ + line.c_, 0)) {
        if (equal(next.x * line.a_ + next.y * line.b_ + line.c_, 0)) {
          if (between(cur, next, point)) {
            return true;
          }
          if (point.x <= cur.x) {
            ++amount;
            ++i;
          }
          continue;
        }
        auto prev = vert(i - 1);
        if (cur.x < point.x) {
          continue;
        }
        if (equal(cur.x, point.x)) {
          return true;
        }
        if ((next.x * line.a_ + next.y * line.b_ + line.c_ > 0 &&
             prev.x * line.a_ + prev.y * line.b_ + line.c_ < 0) ||
            (next.x * line.a_ + next.y * line.b_ + line.c_ < 0 &&
             prev.x * line.a_ + prev.y * line.b_ + line.c_ > 0)) {
          ++amount;
        } else {
          continue;
        }
      } else {
        if (type(line, Line(cur, next)) > 0) {
          Point cross = intersect(line, Line(cur, next));
          if (cross.x >= point.x &&
          between(cur, next, cross) && cross != cur && cross != next) {
            ++amount;
          }
        }
      }
    }
    return (amount % 2 == 1);
  }

  void rotate(const Point& center, double angle) override {
    double rad = deg_to_rad(angle);
    for (int i = 0; i < verticesCount(); ++i) {
      vertices_[i].rotate(center, rad);
    }
  }

  void reflect(const Point& center) override {
    for (int i = 0; i < verticesCount(); ++i) {
      vertices_[i].reflect(center);
    }
  }

  void reflect(const Line& axis) override {
    for (int i = 0; i < verticesCount(); ++i) {
      vertices_[i].reflect(axis);
    }
  }

  void scale(const Point& center, double coefficient) override {
    for (int i = 0; i < verticesCount(); ++i) {
      vertices_[i].scale(center, coefficient);
    }
  }
};


class Ellipse : public Shape {
  Point first_;
  Point second_;

 protected:
  double distance_;

  vector<double> abc() const {
    double a = distance_ / 2;
    double b = sqrt(pow(a, 2) - pow(dist(first_, second_) / 2, 2));
    double c = sqrt(pow(a, 2) - pow(b, 2));
    return {a, b, c};
  }

 public:
  pair<Point, Point> focuses() const {
    return make_pair(first_, second_);
  }

  pair<Line, Line> directrices() const {
    auto params = abc();
    Line line = Line(first_, second_);
    Point d_1 = first_;
    d_1.scale(second_, (params[0] + params[0] * params[0] / params[2]) /(2 * params[0]));
    Point d_2 = second_;
    d_2.scale(first_, (params[0] + params[0] * params[0] / params[2]) / (2 * params[0]));
    return {Line(d_1, d_1 + Point(line.a_, line.b_)),
            Line(d_2, d_2 + Point(line.a_, line.b_))};
  }

  double eccentricity() const {
    auto params = abc();
    return params[2] / params[0];
  }

  Point center() const {
    return {(first_.x + second_.x) / 2, (first_.y + second_.y) / 2};
  }

  Ellipse (Point first, Point second, double distance) :
  first_(first), second_(second), distance_(distance) {}

  double perimeter() override {
    auto params = abc();
    return M_PI * (3 * (params[0] + params[1]) -
    sqrtl((3 * params[0] + params[1]) * (params[0] + 3 * params[1])));
  }

  double area() const override {
    auto params = abc();
    return M_PI * params[0] * params[1];
  }

  bool operator==(const Shape& another) const override {
    auto casted = dynamic_cast<const Ellipse*>(&another);
    if ((casted != nullptr) &&
        ((casted->first_ == first_ && casted->second_ == second_) ||
        (casted->first_ == second_ && casted->second_ == first_)) &&
        equal(casted->distance_, distance_)) {
      return true;
    }
    return false;
  }

  bool isCongruentTo(const Shape& another) const override {
    auto casted = dynamic_cast<const Ellipse*>(&another);
    if ((casted != nullptr) &&
        equal(dist(casted->first_, casted->second_),
              dist(first_, second_)) &&
        equal(casted->distance_, distance_)) {
      return true;
    }
    return false;
  }

  bool isSimilarTo(const Shape& another) const override {
    auto casted = dynamic_cast<const Ellipse*>(&another);
    if ((casted != nullptr) &&
        equal(dist(casted->first_, casted->second_),
              dist(first_, second_) * casted->distance_ / distance_)) {
      return true;
    }
    return false;
  }

  bool containsPoint(const Point& point) const override {
    if (dist(point, first_) + dist(point, second_) <= distance_) {
      return true;
    }
    return false;
  }

  void rotate(const Point& center, double angle) override {
    double rad = deg_to_rad(angle);
    first_.rotate(center, rad);
    second_.rotate(center, rad);
  }

  void reflect(const Point& center) override {
    first_.reflect(center);
    second_.reflect(center);
  }

  void reflect(const Line& axis) override {
    first_.reflect(axis);
    second_.reflect(axis);
  }

  void scale(const Point& center, double coefficient) override {
    first_.scale(center, coefficient);
    second_.scale(center, coefficient);
    distance_ *= coefficient;
  }
};


class Circle : public Ellipse {
 public:
  pair<Line, Line> directrices() const = delete;

  double eccentricity() const { return 0; }

  double radius() const {
    return distance_ / 2;
  }

  Circle(Point point, double radius) : Ellipse(point, point, radius * 2) {}
};


class Rectangle : public Polygon {
 public:
  Point center() const {
    vector<Point> ver = getVertices();
    return {(ver[0].x + ver[2].x) / 2, (ver[0].y + ver[2].y) / 2};
  }

  std::pair<Line, Line> diagonals() const {
    vector<Point> ver = getVertices();
    return {Line(ver[0], ver[2]), Line(ver[1], ver[3])};
  }

  Rectangle(Point first, Point second, double k) :
  Polygon(first, Point(0, 0), second, Point(0, 0)) {
    double angle = M_PI - 2 * atan(k);
    Point middle = Point(first, second);
    first.rotate(middle, angle);
    vertices_[1] = first;
    second.rotate(middle, angle);
    vertices_[3] = second;
  }
};


class Square : public Rectangle {
 public:
  Circle circumscribedCircle() const {
    vector<Point> ver = getVertices();
    return {center(), dist(ver[0], ver[1]) / 2};
  }

  Circle inscribedCircle() const {
    vector<Point> ver = getVertices();
    return {center(), dist(ver[0], ver[2]) / 2};
  }

  Square(Point first, Point second) : Rectangle(first, second, 1) {}
};


class Triangle : public Polygon {
  using Polygon::Polygon;

 public:
  Circle circumscribedCircle() const {
    Line first = orthogonal(Line(vertices_[0], vertices_[1]),
                            Point((vertices_[0].x + vertices_[1].x) / 2,
                                  (vertices_[0].y + vertices_[1].y) / 2));
    Line second = orthogonal(Line(vertices_[1], vertices_[2]),
                             Point((vertices_[1].x + vertices_[2].x) / 2,
                                   (vertices_[1].y + vertices_[2].y) / 2));
    Point center = intersect(first, second);
    return {center, dist(center, vertices_[0])};
  }

  Circle inscribedCircle() const {
    double a = dist(vertices_[0], vertices_[1]);
    double b = dist(vertices_[1], vertices_[2]);
    double c = dist(vertices_[2], vertices_[0]);
    Point tmp;
    tmp.x = (vertices_[2].x * a + vertices_[0].x * b + vertices_[1].x * c) / (a + b + c);
    tmp.y = (vertices_[2].y * a + vertices_[0].y * b + vertices_[1].y * c) / (a + b + c);
    return {tmp, 2 * area() / (a + b + c)};
  }

  Point centroid() const {
    double x = (vertices_[0].x + vertices_[1].x + vertices_[2].x) / 3;
    double y = (vertices_[0].y + vertices_[1].y + vertices_[2].y) / 3;
    return {x, y};
  }

  Point orthocenter() const {
    Line first = orthogonal(Line(vertices_[0], vertices_[1]), vertices_[2]);
    Line second = orthogonal(Line(vertices_[1], vertices_[2]), vertices_[0]);
    return intersect(first, second);
  }

  Line EulerLine() const {
    return {circumscribedCircle().center(), orthocenter()};
  }

  Circle ninePointsCircle() const {
    Point first = circumscribedCircle().center();
    Point second = orthocenter();
    Point center = Point((first.x + second.x) / 2, (first.y + second.y) / 2);
    return {center, circumscribedCircle().radius()/2};
  }
};