#include "AlignedStorage.h"
#include "result.h"
#include <type_traits>

int main() {
    sys::aligned_storage<int> a;
    a.data<const int>();
    
    sys::result<int, const int> res(sys::error_tag, 42);
    int x = 1;
    sys::result<int, int&> res2(sys::error_tag, x);
}
