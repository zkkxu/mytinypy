#include "type.c"

int main(int argc, char *argv[])
{
    VmType *tp = tp_init(argc, argv);

    ezCall(tp, "py2bc", "tinypy", NONE);
    tp_deinit(tp);
    return (0);
}
