#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> // for basename
#include <unistd.h> // for getcwd

#include "../include/installer.h"
#include "../include/parser.h"
#include "../include/resolver.h"
#include "../include/uninstaller.h"
#include "../include/util.h"
#include "../include/commands.h"

#define PATH_MAX 4096

void print_version();

typedef enum
{
    CMD_MAKE,
    CMD_REMOVE,
    CMD_SHOW,
    CMD_SHOW_ENV,
    CMD_UPDATE_DB,
    CMD_HELP,
    CMD_VERSION,
    CMD_UNKNOWN
} Command;

Command parse_command(int argc, char *argv[])
{
    char *cmd = argv[1];
    char *env_name = "";

    if (argc > 2)
        env_name = argv[2];

    if (strcmp(cmd, "make") == 0)
        return CMD_MAKE;
    if (strcmp(cmd, "remove") == 0)
        return CMD_REMOVE;
    if (strcmp(cmd, "show") == 0)
    {
        if (strcmp(env_name, "") == 0)
            return CMD_SHOW;
        else
            return CMD_SHOW_ENV;
    }
    if (strcmp(cmd, "update-db") == 0)
        return CMD_UPDATE_DB;
    if (strcmp(cmd, "--help") == 0 || strcmp(cmd, "help") == 0)
        return CMD_HELP;
    if (strcmp(cmd, "--version") == 0 || strcmp(cmd, "version") == 0)
        return CMD_VERSION;
    return CMD_UNKNOWN;
}

// ~/.local/share/virt-pack for persistent files
// ~/.cache/virt-pack for non-critical logs or intermediate files

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_help();
        return 1;
    }

    Command cmd = parse_command(argc, argv);

    switch (cmd)
    {
    case CMD_HELP:
        print_help();
        break;

    case CMD_VERSION:
        print_version();
        break;

    case CMD_MAKE:
    {
        char env_name[256];

        if (argc >= 3)
        {
            snprintf(env_name, sizeof(env_name), "%s", argv[2]);
        }
        else
        {
            printf("Usage: virt-pack make <env-name>");
            return 1;
        }
        // logic to run parser -> resolver -> installer whilst referencing events.jsonl from the user's project directory
        // ( after running bear intercept -- make )
        handle_make(argc, argv);
        break;
    }

    case CMD_REMOVE:
    {
        char env_name[256]; // buffer to hold the directory path

        if (argc >= 3)
        {
            snprintf(env_name, sizeof(env_name), "%s", argv[2]);
        }
        else
        {
            printf("Usage: virt-pack remove <env-name>\n");
            printf("Use `virt-pack show` to show names of all environments.\n");
            return 1;
        }
        handle_remove(argc, argv);
        break;
    }

    case CMD_SHOW:
    {
        if (show_environments() < 0)
            return 1;
        break;
    }

    case CMD_SHOW_ENV:
    {
        char env_name[256];

        if (argc >= 3)
        {
            snprintf(env_name, sizeof(env_name), "%s", argv[2]);
        }

        show_env_packages(env_name);
        break;
    }

    case CMD_UPDATE_DB:
    {
        int result = system("python3 /usr/local/share/virt-pack/scripts/update_db.py");
        if (result != 0)
        {
            fprintf(stderr, "[ERROR] Failed to update database\n");
        }
        break;
    }

    case CMD_UNKNOWN:
    {
        printf("Command not recognized. Use `virt-pack help` to see valid commands.");
        break;
    }
    }
}

void print_version()
{
    const char *version_text =
        "virt-pack version 1.0.0\n";

    fputs(version_text, stdout);
}
