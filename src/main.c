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

#define VIRT_PACK_LOCAL_DIR_PATH ".local/share/virt-pack"
#define DATA_DIR "data/"
#define INSTALLED_FILE DATA_DIR "installed.json"
#define PATH_MAX 4096

void print_help();
void print_version();
int show_environments();
void make(int argc, char *argv[]);
void bear_intercept();

typedef enum
{
    CMD_MAKE,
    CMD_REMOVE,
    CMD_SHOW,
    CMD_UPDATE_DB,
    CMD_HELP,
    CMD_VERSION,
    CMD_UNKNOWN
} Command;

Command parse_command(const char *cmd)
{
    if (strcmp(cmd, "make") == 0)
        return CMD_MAKE;
    if (strcmp(cmd, "remove") == 0)
        return CMD_REMOVE;
    if (strcmp(cmd, "show") == 0)
        return CMD_SHOW;
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

    Command cmd = parse_command(argv[1]);

    switch (cmd)
    {
    case CMD_HELP:
        print_help();
        break;

    case CMD_VERSION:
        print_version();
        break;

    case CMD_MAKE:
        char env_name[256]; // buffer to hold the directory path

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
        // ( after running intercept -- make )
        make(argc, argv);
        break;

    case CMD_REMOVE:
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
        break;

    case CMD_SHOW:
        if (show_environments() < 0)
            return 1;
        break;

    case CMD_UPDATE_DB:
        int result = system("python3 scripts/update_db.py");
        if (result != 0)
        {
            fprintf(stderr, "[ERROR] Failed to update database\n");
        }

    case CMD_UNKNOWN:
        printf("Command not recognized. Use `virt-pack help` to see valid commands.");
        break;
    }
}

void print_help()
{
    const char *help_text =
        "\n"
        "Usage:\n"
        "   virt-pack make <env-name>\n"
        "   virt-pack remove <env-name>\n"
        "   virt-pack show\n"
        "   virt-pack --version | virt-pack version\n"
        "   virt-pack --help | virt-pack help\n";

    fputs(help_text, stdout);
}

void print_version()
{
    const char *version_text =
        "virt-pack version 1.0.0\n";

    fputs(version_text, stdout);
}

int show_environments()
{
    json_error_t error;
    json_t *root = json_load_file(INSTALLED_FILE, 0, &error);

    if (!root || !json_is_object(root))
    {
        fprintf("[ERROR] Could not load or parse %s: %s\n", INSTALLED_FILE, error.text);
        return -1;
    }

    const char *key;
    json_t *value;
    int count = 0;

    printf("Finding created environments...");
    json_object_foreach(root, key, value)
    {
        printf("%s  ", key);
        count++;
    }
    printf("\n");

    if (count == 0)
        printf("    (No environments found)\n");

    json_decref(root);
    return 0;
}

void bear_intercept()
{
    printf("Intercepting using bear...\n");

    // get local directory path
    char local_dir[PATH_MAX];
    get_local_dir(local_dir, sizeof(local_dir));

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "intercept -- make > %s/events.jsonl", local_dir);

    int ret = system(cmd);
    if (ret != 0)
    {
        fprintf(stderr, "[ERROR] Failed to run bear intercept command\n");
    }
}

void make(int argc, char *argv[])
{
    bear_intercept();
    parser_main();
    resolver_main();
    installer_main(argc, argv);
}