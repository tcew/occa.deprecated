#ifndef __OCCA_PODS
#define __OCCA_PODS

class occaDim {
public:
  union {
    struct {
      size_t x, y, z;
    };
    size_t data[3];
  };

  inline occaDim() :
    x(1),
    y(1),
    z(1) {}

  inline occaDim(size_t x_) :
    x(x_),
    y(1),
    z(1) {}

  inline occaDim(size_t x_, size_t y_) :
    x(x_),
    y(y_),
    z(1) {}

  inline occaDim(size_t x_, size_t y_, size_t z_) :
    x(x_),
    y(y_),
    z(z_) {}

  inline occaDim(const occaDim &d) :
    x(d.x),
    y(d.y),
    z(d.z) {}

  inline occaDim& operator = (const occaDim &d){
    x = d.x;
    y = d.y;
    z = d.z;

    return *this;
  }

  size_t& operator [] (int i){
    return data[i];
  }
};

#endif
