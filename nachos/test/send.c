#include "userlib/syscall.h"
#include "userlib/libnachos.h"

int main() {
    n_printf("Je suis le sender\n");
    char* message = "Hello world!\0";
    // On envoie le message "Hello world!" sur le Bus Serie ACIA
    n_printf("Message envoyé: %s\n", message);
    n_printf("Taille du message envoyé: %d\n", TtySend(message));

    // On eteind la machine
    Halt();

    return 0;
}
