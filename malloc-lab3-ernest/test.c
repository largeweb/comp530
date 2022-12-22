/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
    printf("Start malloc\n");
    void *x = malloc(1);
    printf("Malloc @: %p\n", x);
    void *y = malloc(3);
    printf("Malloc @: %p\n", y);
    free(y);
    void *z = malloc(3);
    printf("Malloc @: %p\n", z);
    
    return (errno);
}
