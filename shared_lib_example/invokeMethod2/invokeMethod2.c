/*
 * invokeMethod2.c
 *
 *  Created on: Apr 3, 2018
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main(int *argc, char **argv)
{
    void *fHandle;
    void (*func)();

    fHandle = dlopen("../../testSharedLib/Debug/libtestSharedLib.so",RTLD_LAZY);

    if (!fHandle) {
        fprintf (stderr, "%s\n", dlerror());
        exit(1);
    }
    dlerror();

    func = (void(*)())dlsym(fHandle,"bejo_lib");

    if (func) {
        func("BEJO", 999);
    }

    dlclose(fHandle);
    return 0;
}
