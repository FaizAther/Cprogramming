#include <criterion/criterion.h>

#include "../src/s.h"

Test(s_test, line_create)
{
    // test contents
    string my_s = init_line ();
    cr_assert(my_s.data == NULL && my_s.len == 0);
}
