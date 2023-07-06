#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PROGRAM_NAME "reveal"
#define PROGRAM_LICENSE "Copyright (c) 2023, Sherman Rofeman. MIT license."
#define PROGRAM_VERSION "v5.2.0"

#define GIGA 1e9
#define MEGA 1e6
#define KILO 1e3

#define PARSE_METADATA_FLAG(flag, function, argument)                          \
    if (!strcmp(flag, argument))                                               \
    {                                                                          \
        function;                                                              \
        exit(EXIT_SUCCESS);                                                    \
    }
#define PARSE_SIZE(buffer, metadata, multiplier, unit)                         \
    buffer = metadata->st_size / (multiplier);                                 \
    if ((int)buffer)                                                           \
    {                                                                          \
        printf("%.1f%s\n", buffer, unit);                                      \
        return;                                                                \
    }
#define CASE_FUNCTION(value, function)                                         \
    case (value):                                                              \
        function;                                                              \
        break;
#define CASE_PUTS(value, text) CASE_FUNCTION(value, puts(text))

uint8_t exitCode = EXIT_SUCCESS;

void PrintHelp()
{
    printf("Usage: %s [FLAGS]... [PATHS]...\n", PROGRAM_NAME);
    puts("Reveals information about entries in the file system.");
    puts("");
    puts("METADATA FLAGS");
    puts("These flags show metadata about the program.");
    puts("");
    puts("  --help     prints these help instructions.");
    puts("  --version  prints its version.");
    puts("  --license  prints its license.");
    puts("");
    puts("DATA TYPE FLAGS");
    puts("These flags change what data type the program will reveal from the "
         "entries.");
    puts("");
    puts("  --contents (default)  prints its contents.");
    puts("  --type                prints its type.");
    puts("  --size                prints its size in bytes.");
    puts("  --human-size          prints its size using the most readable "
         "unit.");
    puts("  --user                prints the user that owns it.");
    puts("  --user-id             prints the ID of the user that owns it.");
    puts("  --group               prints the group that owns it.");
    puts("  --group-id            prints the ID of the group that owns it.");
    puts("  --permissions         prints its permissions in octal base.");
    puts("  --human-permissions   prints its permissions for user, group "
         "and others,");
    puts("                        respectively, using three set of characters "
         "each:");
    puts("                        read (r), write (w), execute (x) and lack "
         "(-).");
    puts("  --inode               prints its serial number.");
    puts("  --modified-date       prints the date where its contents were "
         "last modified.");
    puts("");
    puts("TRANSPASSING FLAGS");
    puts("These flags changes the way the symlinks must be handled.");
    puts("");
    puts("  --untranspass (default)  does not resolve symlinks.");
    puts("  --transpass              resolves all levels of symlinks.");
    puts("");
    puts("EXIT CODES");
    puts("It will throw exit code 1 in the end of its execution if an "
         "error happened.");
    puts("");
    puts("However, while still able to continue, it will keep revealing the "
         "remaining");
    puts("arguments.");
    puts("");
    puts("All the errors found will be reported through stderr.");
    puts("");
    puts("SOURCE CODE");
    puts("Its source code is available at:");
    puts("  https://github.com/skippyr/reveal");
    puts("");
    puts("ISSUES");
    puts("Report issues found in this program at:");
    puts("  https://github.com/skippyr/reveal/issues");
}

void PrintComposedError(const char *const descriptionStart,
                        const char *const descriptionMiddle,
                        const char *const descriptionEnd)
{
    fprintf(stderr, "%s: %s%s%s\n", PROGRAM_NAME, descriptionStart,
            descriptionMiddle, descriptionEnd);
    exitCode = EXIT_FAILURE;
}

void RevealType(const struct stat *const metadata)
{
    switch (metadata->st_mode & S_IFMT)
    {
        CASE_PUTS(S_IFBLK, "Block")
        CASE_PUTS(S_IFCHR, "Character")
        CASE_PUTS(S_IFDIR, "Directory")
        CASE_PUTS(S_IFIFO, "Fifo")
        CASE_PUTS(S_IFLNK, "Symlink")
        CASE_PUTS(S_IFREG, "File")
        CASE_PUTS(S_IFSOCK, "Socket");
    default:
        puts("Unknown");
    }
}

void RevealHumanSize(const struct stat *const metadata)
{
    float size;
    PARSE_SIZE(size, metadata, GIGA, "GB")
    PARSE_SIZE(size, metadata, MEGA, "MB")
    PARSE_SIZE(size, metadata, KILO, "KB")
    printf("%ldB\n", metadata->st_size);
}

void RevealUser(const struct stat *const metadata, const char *const path)
{
    const struct passwd *const owner = getpwuid(metadata->st_uid);
    if (owner)
        puts(owner->pw_name);
    else
        PrintComposedError("could not get user that owns \"", path, "\".");
}

void RevealGroup(const struct stat *const metadata, const char *const path)
{
    const struct group *const group = getgrgid(metadata->st_gid);
    if (group)
        puts(group->gr_name);
    else
        PrintComposedError("could not get group that owns \"", path, "\".");
}

void RevealFile(const char *const path)
{
    FILE *const file = fopen(path, "r");
    if (!file)
    {
        PrintComposedError("could not open file \"", path, "\".");
        return;
    }
    char character;
    while ((character = fgetc(file)) != EOF)
        putchar(character);
    fclose(file);
}

void RevealDirectory(const char *const path)
{
    char absolutePath[PATH_MAX];
    if (!realpath(path, absolutePath))
    {
        PrintComposedError("could not resolve absolute path of \"", path,
                           "\".");
        return;
    }
    DIR *const directory = opendir(path);
    if (!directory)
    {
        PrintComposedError("could not open directory \"", path, "\".");
        return;
    }
    const char *const separator = !strcmp(absolutePath, "/") ? "" : "/";
    const struct dirent *entry;
    while ((entry = readdir(directory)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        printf("%s%s%s\n", absolutePath, separator, entry->d_name);
    }
    closedir(directory);
}

void Reveal(const char *const path, const uint8_t dataType,
            const uint8_t isTranspassing)
{
    struct stat metadata;
    if (isTranspassing ? stat(path, &metadata) : lstat(path, &metadata))
    {
        PrintComposedError("the path \"", path,
                           "\" does not points to anything.");
        return;
    }
    switch (dataType)
    {
        CASE_FUNCTION(1, RevealType(&metadata))             // --type
        CASE_FUNCTION(2, printf("%ld\n", metadata.st_size)) // --size
        CASE_FUNCTION(3, RevealHumanSize(&metadata))        // --human-size
        CASE_FUNCTION(4, RevealUser(&metadata, path))       // --user
        CASE_FUNCTION(5, printf("%u\n", metadata.st_uid))   // --user-uid
        CASE_FUNCTION(6, RevealGroup(&metadata, path))      // --group
        CASE_FUNCTION(7, printf("%u\n", metadata.st_gid))   // --group-id
    default:                                                // --contents
        switch (metadata.st_mode & S_IFMT)
        {
            CASE_FUNCTION(S_IFREG, RevealFile(path))
            CASE_FUNCTION(S_IFDIR, RevealDirectory(path))
        default:
            PrintComposedError("can not reveal the contents of \"", path,
                               "\" type.");
        }
    }
}

int main(int quantityOfArguments, const char **arguments)
{
    for (int i = 1; i < quantityOfArguments; i++)
    {
        PARSE_METADATA_FLAG("--license", puts(PROGRAM_LICENSE), arguments[i]);
        PARSE_METADATA_FLAG("--help", PrintHelp(), arguments[i]);
        PARSE_METADATA_FLAG("--version", puts(PROGRAM_VERSION), arguments[i]);
    }
    const char *dataTypeFlags[] = {"--contents",    "--type",
                                   "--size",        "--human-size",
                                   "--user",        "--user-id",
                                   "--group",       "--group-id",
                                   "--permissions", "--human-permissions",
                                   "--inode",       "--modified-date"};
    uint8_t dataType = 0, isTranspassing = 0;
    for (int i = 1; i < quantityOfArguments; i++)
    {
        for (uint8_t j = 0; j < sizeof(dataTypeFlags) / sizeof(NULL); j++)
        {
            if (!strcmp(dataTypeFlags[j], arguments[i]))
            {
                dataType = j;
                goto end;
            }
        }
        if (!strcmp("--transpass", arguments[i]))
            isTranspassing = 1;
        else if (!strcmp("--untranspass", arguments[i]))
            isTranspassing = 0;
        else
            Reveal(arguments[i], dataType, isTranspassing);
    end:;
    }
    return exitCode;
}
