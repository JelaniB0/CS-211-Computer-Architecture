#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pa3.h"
#include "pa3_test.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // use this struct declaration to define your test files and the test functions to run them with
	// the filesystems folder contains more filesystems than are listed here, and you
	// should likely make use of all or most of them (you can replace their names below)
	// you can also add more entries and more functions in the same pattern as those that already exist
    struct filesystem_test_config cfgs[] = {
        {"filesystems/fs9_root_with_max_dirs_with_max_files_each_all_rw.bin"        , fsB_test},
        // {"filesystems/fs4_root_with_two_files_both_rw.bin"                        , fs4_test},
        // {"filesystems/fs2_root_only_w.bin"                                        , fs2_test},
        // {"filesystems/fs3_root_only_no_permissions.bin"                           , fs3_test},
    };
    
    for (unsigned int cfg_idx = 0; cfg_idx < sizeof(cfgs) / sizeof(cfgs[0]); cfg_idx++) {
        struct filesystem_test_config *cfg = &cfgs[cfg_idx];
        printf("[INFO] testing filesystem: \"%s\"\n", cfg->serialized_filename);

        /* read the file from disk */
        void *file_contents = read_serialized_filesystem_file(cfg->serialized_filename);
        if(file_contents == NULL) {
            report_test_result(TEST_RESULT_UNABLE_TO_OPEN_TEST_FILE);
            break;
        }

        /* deserialize the file */
        struct fs_node *deserialized_root = NULL;
        enum error_code ec = pa3_deserialize_node(&deserialized_root, 0, file_contents);
        if(ec != ERROR_NONE) {   
            report_test_result(TEST_RESULT_FAIL_TO_DESERIALIZE_FILESYSTEM);
            free(file_contents);
            break;
        }

        /* run the testing function corresponding to this file */
        enum test_result tr = cfg->test_func(deserialized_root);
        report_test_result(tr);

        free(file_contents);   

        if(tr != TEST_RESULT_PASS) {
            printf("[INFO] Stopping due to failed test\n");
            break;
        }    
    }

    return EXIT_SUCCESS;
}
