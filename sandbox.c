#include <stdio.h>

typedef void (*operation_ptr)(void);
void say_hello(void)
{
    printf("%s\n", "hello world");
}

typedef struct Speaker 
{
    operation_ptr say;
} Speaker;

int main(void)
{
    Speaker speaker;
    speaker.say = say_hello;
    speaker.say();

    printf("Address of main: %p\n",main + 0x10);
    printf("Address of say_hello: %p\n",say_hello);
    return 0;
}
