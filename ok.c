#include <string.h>
#include <fcntl.h>
#include <unistd.h>



int main() {
    int fd = open("ok.txt", O_RDWR | O_CREAT);
    char *ok = "sadfasdasdasddsa\r\nasdasdasdasdasdasdasdasdasd\r\n\r\n\r\n\r\n\r\n\r\n\r\n";
    write(fd, ok, strlen(ok));

}