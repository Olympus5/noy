#include "userlib/syscall.h"
#include "userlib/libnachos.h"

int main() {
    n_printf("Je suis le sender\n");

    // On envoie le message "Hello world!" sur le Bus Serie ACIA
    n_printf("Taille du message envoyé: %d\n", TtySend("Hello world!\0"));

    // On eteind la machine
    Halt();

    return 0;
}
