#include "userlib/syscall.h"
#include "userlib/libnachos.h"

int main() {
    int length = 256;
    char message[length];
    char* message2 = "Bonjour le monde";
    n_printf("Je suis le receiver\n");

    n_printf("Taille du message reçue: %d\n", TtyReceive(message, length));
    n_printf("Message reçu: %s\n", message);

    n_printf("Message envoyé: %s\n", message2);
    n_printf("Taille du message envoyé: %d\n", TtySend(message2));
    // On eteind la machine
    Halt();

    return 0;
}
