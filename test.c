#include <stdio.h>
#include <dlfcn.h>

#include "mach_hook.h"

void libtest();  //from libtest.dylib

int hooked_puts(char const *s)
{
    puts(s);  //calls the original puts() from libSystem.B.dylib, because our main executable module called "test" remains intact

    return puts("HOOKED!");
}

int main()
{
    void *handle = 0;  //handle to store hook-related info
    mach_substitution original;  //original data for restoration
	Dl_info info;

	if (!dladdr((void const *)libtest, &info))  //gets an address and a path of a target library
    {
        fprintf(stderr, "Failed to get the base address of a library at `%s`!\n", info.dli_fname);

        goto end;
    }

    handle = mach_hook_init(info.dli_fname, info.dli_fbase);

    if (!handle)
    {
        fprintf(stderr, "Redirection init failed!\n");

        goto end;
    }

    libtest();  //calls puts() from libSystem.B.dylib

    puts("-----------------------------");

    original = mach_hook(handle, "puts", (mach_substitution)hooked_puts);

    if (!original)
    {
        fprintf(stderr, "Redirection failed!\n");

        goto end;
    }

    libtest();  //calls hooked_puts()

    puts("-----------------------------");

    original = mach_hook(handle, "puts", original);  //restores the original relocation

    if (!original)
    {
        fprintf(stderr, "Restoration failed!\n");

        goto end;
    }

    libtest();  //again calls puts() from libSystem.B.dylib

end:

    mach_hook_free(handle);
    handle = 0;  //no effect here, but just a good advice to prevent double freeing

    return 0;
}
