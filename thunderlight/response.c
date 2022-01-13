#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "response.h"


typedef struct {
    const char *message;
    const char *code;
    uint8_t length;
} StatusMessage;

bool setup = false;

StatusMessage messages[511] = {};

void StatusMessage_setup(void) {
    messages[100].message = "Continue";
    messages[100].code = "100";
    messages[100].length = 8;
    messages[200].message = "OK";
    messages[200].code = "200";
    messages[200].length = 2;
    messages[404].message = "Not Found";
    messages[404].code = "404";
    messages[404].length = 9;
    setup = true;
}

void Response_init(Response *self) {
    self->code = 200;
    HeaderMap_init(&(self->headers));
    self->body = NULL;
    self->body_len = 0;
    Buffer_init(&(self->buffer));
}

void Response_set_version_info(Response *self, char *version, uint8_t version_len) {
    self->version = version;
    self->version_len = version_len;
}

char *Response_get_header_bytes(Response *self, size_t *len) {
    if (!setup) {
        StatusMessage_setup();
    }
    printf("WILL SET VERSION\n");
    fflush(stdout);
    Buffer_append(&(self->buffer), self->version, self->version_len);
    Buffer_append(&(self->buffer), " ", 1);
    printf("WILL SET CODE %s\n", messages[self->code].message);
    fflush(stdout);
    Buffer_append(&(self->buffer), messages[self->code].code, 3);
    printf("WILL SET CODE 2\n");
    fflush(stdout);
    Buffer_append(&(self->buffer), " ", 1);
    printf("WILL SET CODE 3\n");
    fflush(stdout);
    Buffer_append(&(self->buffer), messages[self->code].message, messages[self->code].length);
    printf("WILL SET CODE 4\n");
    fflush(stdout);
    Buffer_append(&(self->buffer), "\r\n\r\n", 4);
    printf("WILL SET DATE\n");
    fflush(stdout);
    char date[64];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    size_t date_len = strlen(date);
    Buffer_append(&(self->buffer), "Date: ", 6);
    Buffer_append(&(self->buffer), date, date_len);
    Buffer_append(&(self->buffer), "\r\n", 2);
    Buffer_append(&(self->buffer), "Server: Thunderlight 0.2.0\r\n", 28);
    // examine each header here
    printf("HERE RUN\n");
    fflush(stdout);
    for (size_t i = 0; i < self->headers.len; i++) {
        Buffer_append(&(self->buffer), self->headers.buffer[i].key, self->headers.buffer[i].key_len);
        Buffer_append(&(self->buffer), ": ", 2);
        Buffer_append(&(self->buffer), self->headers.buffer[i].value, self->headers.buffer[i].value_len);
        Buffer_append(&(self->buffer), "\r\n", 2);
    }
    printf("AFTER FOR\n");
    fflush(stdout);
    Buffer_append(&(self->buffer), "Connection: close\r\n", 19);
    Buffer_append(&(self->buffer), "Content-Length: ", 16);
    char content_len[16];
    snprintf(content_len, 16, "%d", self->body_len);
    Buffer_append(&(self->buffer), content_len, strlen(content_len));
    Buffer_append(&(self->buffer), "\r\n\r\n", 4);
    printf("AFTER ALL\n");
    fflush(stdout);
    *len = self->buffer.length;
    return self->buffer.loc;
}

char *Response_get_body_bytes(Response *self, size_t *len) {
    self->body = "{\"key\": \"value\"}";
    *len = self->body_len;
    return self->body;
}
