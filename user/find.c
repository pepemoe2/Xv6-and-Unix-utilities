#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
char * get_filename(char* path)
{
    char* p;
    for(p= path + strlen(path); p >= path && *p != '/'; p-- );
    return p + 1;
}
void search_file_recursive(char *dir_path, char *target_name)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if (!strcmp(get_filename(dir_path), target_name))
    {
        printf("%s\n", dir_path);
    }
    if((fd = open(dir_path, O_RDONLY)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", dir_path);
        return;
    }

    if(fstat(fd,&st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", dir_path);
        close(fd);
        return;
    }
    if(st.type != T_DIR)
    {
        close(fd);
        return;
    }
    memset(buf, 0, sizeof(buf));  // Xóa bộ nhớ trước khi dùng
    strcpy(buf, dir_path);
    p = buf + strlen(buf);
    *p++ = '/';

    while(read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if(de.inum == 0) continue;

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;  // Đảm bảo chuỗi kết thúc bằng '\0'

        if(!strcmp(de.name, ".") || !strcmp(de.name, "..")) continue;

        search_file_recursive(buf, target_name);
    }
    close(fd);
}
int main (int argc, char* argv[])
{
    if(argc < 3){
        fprintf(2,"usage: find path filename\n"); // hướng dẫn sử dụng
        exit(1);
    }
    
    search_file_recursive(argv[1],argv[2]);
    exit(0);
}