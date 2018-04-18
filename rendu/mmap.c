#include "userlib/syscall.h"
#include "userlib/libnachos.h"

#define FILE "val.txt"
#define SIZE 16

int main() {
    int* a;
    int i;
    OpenFileId fid = Open(FILE);

    a = Mmap(fid,SIZE);

    for(i = 0; i < SIZE; i++) {
        n_printf("%d\n", a[i]);
    }

    Close(fid);

    return 0;
}
