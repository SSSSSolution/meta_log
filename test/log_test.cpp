#include <iostream>


#include "meta_log/logger.h"

void test()
{
     std::cout << TO_STR(META_LOG(app, TRACE, "%s", "123")) << std::endl;

     META_LOG(app, TRACE, "%s", "123")

}

class A
{
public:
    void test(int i = 0)
    {
        META_LOG(app, TRACE, "%d", 1345);
    }
};

int main()
{
//    meta_log::Logger::get_instance()->set_ignored_console_output_optional(0);
    test();
    META_LOG(app, ERROR, "%d", 1345)

    A a;
    a.test();
}
