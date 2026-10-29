// C++ on bare metal: global constructors run before main (Arm: __libc_init_array, RISC-V: rv_start.S walks
// .init_array), virtual dispatch, a template container and placement new into static storage. No heap, no
// exceptions, no RTTI (-fno-exceptions -fno-rtti), so libstdc++ only contributes headers.
#include <new>
#include <stdint.h>
#include "bare.h"

namespace {
int constructed;
struct Registered {
  int id;
  explicit Registered(int i) : id(i) { constructed++; }
};
Registered first(1), second(2);   // constructed before main()

struct Shape {
  virtual int area() const = 0;
 protected:
  ~Shape() = default;             // no virtual destructor: no operator delete pulled in
};
struct Rect final : Shape { int w, h; Rect(int a, int b) : w(a), h(b) {} int area() const override { return w * h; } };
struct Square final : Shape { int s; explicit Square(int a) : s(a) {} int area() const override { return s * s; } };

template <typename T, int N> class Fifo {
  T items[N];
  int head = 0, count = 0;
 public:
  bool push(const T &v) { if (count == N) return false; items[(head + count++) % N] = v; return true; }
  bool pop(T &v) { if (!count) return false; v = items[head]; head = (head + 1) % N; count--; return true; }
};

alignas(Rect) unsigned char rect_storage[sizeof(Rect)];
}  // namespace

extern "C" void __cxa_pure_virtual() { bare_puts("pure virtual call\n"); test_done(); }

int main()
{
  test_check(constructed == 2 && first.id == 1 && second.id == 2, "global constructors ran before main()");
  Rect *r = new (rect_storage) Rect(3, 4);          // placement new: no heap
  Square sq(5);
  const Shape *shapes[] = { r, &sq };
  int total = 0;
  for (const Shape *s : shapes) total += s->area();
  test_check(total == 12 + 25, "virtual dispatch through base pointers");
  Fifo<uint16_t, 4> f;
  for (uint16_t i = 1; i <= 5; i++) f.push(static_cast<uint16_t>(i * 100));
  uint16_t v = 0, sum = 0;
  while (f.pop(v)) sum = static_cast<uint16_t>(sum + v);
  test_check(sum == 100 + 200 + 300 + 400, "template FIFO keeps 4 of 5");
  bare_printf("C++ on %s: areas %d, fifo sum %u\n", BARE_CHIP, total, sum);
  test_done();
}
