#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <string.h>

#define VIRT_PACK_LOCAL_DIR_PATH ".local/share/virt-pack"
#define DATA_DIR "data/"
#define INSTALLED_FILE DATA_DIR "installed.json"

// uninstall a package via apt
void uninstall_package(const char *pkgname)
{
    printf("Uninstalling %s...\n", pkgname);

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo apt remove -y %s", pkgname);

    int ret = system(cmd);
    if (ret != 0)
    {
        fprintf(stderr, "[ERROR] Failed to uninstall package: %s\n", pkgname);
        return;
    }

    printf("[OK] %s uninstalled successfully.\n", pkgname);
}

int uninstaller_main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: virt-pack remove <env-name>");
        return EXIT_FAILURE;
    }

    const char *env_name = argv[2];

    json_error_t error;
    json_t *root = json_load_file(INSTALLED_FILE, 0, &error);

    if (!root || !json_is_array(root))
    {
        fprintf(stderr, "[ERROR] Failed to load %s: %s\n", INSTALLED_FILE, error.text);
        return EXIT_FAILURE;
    }

    json_t *pkg_array = json_object_get(root, env_name);
    if (!pkg_array || !json_is_array(pkg_array))
    {
        fprintf(stderr, "[ERROR] No installed packages found for environment: %s\n", env_name);
        json_decref(root);
        return EXIT_FAILURE;
    }

    size_t i;
    json_t *pkg;
    json_array_foreach(pkg_array, i, pkg)
    {
        if (!json_is_string(pkg))
            continue;

        const char *pkg_name = json_string_value(pkg);
        uninstall_package(pkg_name);
    }

    // remove the environment entry from the file
    json_object_del(root, env_name);
    if (json_dump_file(root, INSTALLED_FILE, JSON_INDENT(2)))
    {
        fprintf(stderr, "[WARNING] Failed to update %s after uninstall\n", INSTALLED_FILE);
    }

    json_decref(root);
    return EXIT_SUCCESS;
}