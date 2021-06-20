#include "client/client.h"
#include "server/server.h"

int main(int args, char *argv[]) {
    if (args < 2) {
        printf("Please set a client or server mode");
        return 0;
    }
    if (strcmp(argv[1], "s") == 0) {
        return serverMode();
    }
    if (strcmp(argv[1], "c") == 0) {
        if (args < 3) {
            printf("Please set a username");
            return 0;
        }
        if (args < 4) {
            printf("Please set a server");
            return 0;
        }
        return clientMode(argv[2], argv[3]);
    }
}
