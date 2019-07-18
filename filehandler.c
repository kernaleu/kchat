#include "common.h"

void file_download(int connfd, int uid, char *buf)
{
    ssize_t bytesread;
    char fname[6] = {0}, path[12];

    for (int i = 1; buf[i] != '\n'; i++) {
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
    FILE *fp = fopen(path, "rb");
    if (fp) {
        while ((bytesread = fread(buf, 1, bufsize, fp)) > 0) {
            write(connfd, buf, bytesread);
        }
        fclose(fp);
    } else {
        server_send(ONLY, uid, " * Couldn't get file descriptor!\n");
    }
    close(connfd);
}

void file_upload(int connfd, int uid, char *buf, ssize_t bytesread)
{
    char fname[6] = {0}, path[12];

    snprintf(fname, 6, "test"); // TODO: Generate filename here.
    snprintf(path, 12, "files/%s", fname);

    FILE *fp = fopen(path, "wb");
    if (fp) {
        server_send(ONLY, uid, "$%s\n", fname);
        fwrite(buf, bytesread, 1, fp);
        while ((bytesread = recv(connfd, buf, bufsize, 0)) > 0) {
            fwrite(buf, 1, bytesread, fp);
        }
        fclose(fp);
    } else {
        server_send(ONLY, uid, " * Couldn't create file.\n");
    }
    close(connfd);
}

