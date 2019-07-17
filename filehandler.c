#include "common.h"

void file_download(int connfd, int uid, char *buf)
{
    ssize_t bytesread;
    char fname[6] = {0}, path[12];

    for (int i = 1; buf[i] != '\n'; i++ ) {
        if (i < 7) {
            fname[i - 1] = buf[i];
        } else {
            server_send(ONLY, uid, " * File id is too large!\n");
            close(connfd);
            return;
        }
    }
    snprintf(path, 12, "files/%s", fname);
    printf(" * path: \"%s\"\n", path);
    int fd = open(path, O_RDONLY);
    if (fd) {
        while ((bytesread = read(fd, buf, bufsize)) > 0) {
            write(connfd, buf, bytesread);
        }
        close(fd);
    } else {
        server_send(ONLY, uid, " * Couldn't get file descriptor!\n");
    }
    close(connfd);
}

void file_upload(int connfd, int uid, char *buf)
{
    ssize_t bytesread;
    char fname[6] = {0}, path[12];

    snprintf(fname, 6, "test"); // TODO: Generate filename here.
    snprintf(path, 12, "files/%s", fname);

    int fd = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd) {
        server_send(ONLY, uid, "$%s\n", fname);
        write(fd, buf, bytesread);
        while ((bytesread = recv(connfd, buf, bufsize, 0)) > 0) {
            write(fd, buf, bytesread);
        }
        close(fd);
    } else {
        server_send(ONLY, uid, " * Couldn't create file.\n");
    }
    close(connfd);
}

