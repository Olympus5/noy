#include "userlib/syscall.h"
#include "userlib/libnachos.h"

int main() {
    int length = 256;
    char message[length];
    n_printf("Je suis le receiver\n");

    n_printf("Taille du message reçue: %d\n", TtyReceive(message, length));
    n_printf("Message reçu: %s", message);

    // On eteind la machine
    Halt();

    return 0;
}
