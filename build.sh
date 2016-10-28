#gcc -Wall  myfuse.c `pkg-config fuse --cflags --libs` -o myfuse -lm
#gcc -Wall  hello.c `pkg-config fuse --cflags --libs` -o hello -lm
gcc -Wall  hello_single.c `pkg-config fuse --cflags --libs` -o hello_single -lm
#gcc -Wall  hello_write.c `pkg-config fuse --cflags --libs` -o hello_write -lm
