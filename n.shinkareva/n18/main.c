#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

// преобразование типа файла и прав доступа в строку
static void make_permissions(mode_t mode, char result[11])
{
    const mode_t bits[9] = {
        S_IRUSR, S_IWUSR, S_IXUSR,
        S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };
    const char symbols[] = "rwxrwxrwx";
    int i;

    result[0] = S_ISDIR(mode) ? 'd' : (S_ISREG(mode) ? '-' : '?');

    for (i = 0; i < 9; i++) {
        result[i + 1] = (mode & bits[i]) ? symbols[i] : '-';
    }

    result[10] = '\0';
}

// вывод: только имя файла, без пути к нему
static void print_short_name(const char *path)
{
    const char *end = path + strlen(path);
    const char *begin;

    // пропускаем завершающие символы /
    while (end > path && end[-1] == '/') {
        end--;
    }

    if (end == path) {
        printf("/");
        return;
    }

    begin = end;

    while (begin > path && begin[-1] != '/') {
        begin--;
    }

    printf("%.*s", (int)(end - begin), begin);
}

int main(int argc, char *argv[])
{
    int i;
    int errors = 0;

    // проверка, переданы ли имена файлов
    if (argc < 2) {
        fprintf(stderr, "usage: %s file...\n", argv[0]);
        return 1;
    }

    // печать заголовка таблицы
    printf("%-10s %5s %-15s %-15s %10s %-16s %s\n",
           "permissions", "links", "owner", "group",
           "size", "modified", "name");

    // обработка каждого переданного пути
    for (i = 1; i < argc; i++) {
        struct stat info;
        struct passwd *owner;
        struct group *group;
        struct tm *time_info;

        char permissions[11];
        char date[32];
        char size[32] = "";
        char owner_number[32];
        char group_number[32];

        const char *owner_name;
        const char *group_name;

        // получение сведений о файле
        if (lstat(argv[i], &info) == -1) {
            perror(argv[i]);
            errors = 1;
            continue;
        }

        // формирую тип файла и права доступа
        make_permissions(info.st_mode, permissions);

        // определение имени владельца по uid
        owner = getpwuid(info.st_uid);

        if (owner != NULL) {
            owner_name = owner->pw_name;
        } else {
            snprintf(owner_number, sizeof(owner_number), "%lu",
                     (unsigned long)info.st_uid);
            owner_name = owner_number;
        }

        // определение имени группы по gid
        group = getgrgid(info.st_gid);

        if (group != NULL) {
            group_name = group->gr_name;
        } else {
            snprintf(group_number, sizeof(group_number), "%lu",
                     (unsigned long)info.st_gid);
            group_name = group_number;
        }

        // размер выводим только для обычных файлов
        if (S_ISREG(info.st_mode)) {
            snprintf(size, sizeof(size), "%lld",
                     (long long)info.st_size);
        }

        // получение даты последнего изменения файла
        time_info = localtime(&info.st_mtime);

        if (time_info == NULL ||
            strftime(date, sizeof(date), "%d.%m.%Y %H:%M",
                     time_info) == 0) {
            strcpy(date, "?");
        }

        // вывод сведений о файле в виде таблицы
        printf("%-10s %5lu %-15s %-15s %10s %-16s ",
               permissions,
               (unsigned long)info.st_nlink,
               owner_name,
               group_name,
               size,
               date);

        print_short_name(argv[i]);
        putchar('\n');
    }

    return errors ? 1 : 0;
}