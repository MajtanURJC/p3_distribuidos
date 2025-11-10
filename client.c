#include STUB_H

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    if (argc != 5) {
        fprintf(stderr, "Not enought arguments\n");
        exit(1);
    }

}