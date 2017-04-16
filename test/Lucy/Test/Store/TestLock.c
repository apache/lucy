/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "charmony.h"

// rmdir
#ifdef CHY_HAS_DIRECT_H
  #include <direct.h>
#endif
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

#include "Lucy/Test/Store/TestLock.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/LockFileLock.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Util/Json.h"

TestLockFileLock*
TestLFLock_new() {
    return (TestLockFileLock*)Class_Make_Obj(TESTLOCKFILELOCK);
}

static Folder*
S_create_fs_folder() {
    Folder *folder = (Folder*)FSFolder_new(SSTR_WRAP_C("_locktest"));
    Folder_Initialize(folder);
    return folder;
}

static void
S_destroy_fs_folder(Folder *folder) {
    Folder_Delete_Tree(folder, SSTR_WRAP_C("locks"));
    rmdir("_locktest");
    DECREF(folder);
}

static void
test_exclusive_only(TestBatchRunner *runner, Lock *lock1, Lock *lock2,
                    const char *tag) {
    TEST_TRUE(runner, Lock_Request_Exclusive(lock1),
              "Request_Exclusive (only) succeeds %s", tag);
    Err_set_error(NULL);
    TEST_FALSE(runner, Lock_Request_Exclusive(lock2),
               "Request_Exclusive (only) after Request_Exclusive fails %s",
               tag);
    TEST_TRUE(runner, CERTIFY(Err_get_error(), LOCKERR),
              "Request_Exclusive (only) after Request_Exclusive sets"
              " LockErr %s", tag);
    Lock_Release(lock1);

    TEST_TRUE(runner, Lock_Request_Exclusive(lock2),
              "Request_Exclusive (only) succeeds after Release %s", tag);
    TEST_FALSE(runner, Lock_Request_Exclusive(lock1),
               "Request_Exclusive (only) fails if lock2 is locked %s", tag);
    DECREF(lock2);

    TEST_TRUE(runner, Lock_Request_Exclusive(lock1),
              "Request_Exclusive (only) succeeds after Destroy %s", tag);
    DECREF(lock1);
}

static void
test_lock(TestBatchRunner *runner, Lock *lock1, Lock *lock2, Lock *lock3,
          const char *tag) {
    TEST_TRUE(runner, Lock_Request_Shared(lock1), "Request_Shared succeeds %s",
              tag);
    TEST_TRUE(runner, Lock_Request_Shared(lock2),
              "Request_Shared on another lock succeeds %s", tag);
    Err_set_error(NULL);
    TEST_FALSE(runner, Lock_Request_Exclusive(lock3),
               "Request_Exclusive after Request_Shared fails %s", tag);
    TEST_TRUE(runner, CERTIFY(Err_get_error(), LOCKERR),
              "Request_Exclusive after Request_Shared sets LockErr %s", tag);
    Lock_Release(lock1);
    Lock_Release(lock2);

    TEST_TRUE(runner, Lock_Request_Exclusive(lock1),
              "Request_Exclusive succeeds %s", tag);
    Err_set_error(NULL);
    TEST_FALSE(runner, Lock_Request_Shared(lock2),
               "Request_Shared after Request_Exclusive fails %s", tag);
    TEST_TRUE(runner, CERTIFY(Err_get_error(), LOCKERR),
              "Request_Shared after Request_Exclusive sets LockErr %s", tag);
    Err_set_error(NULL);
    TEST_FALSE(runner, Lock_Request_Exclusive(lock3),
               "Request_Exclusive after Request_Exclusive fails %s", tag);
    TEST_TRUE(runner, CERTIFY(Err_get_error(), LOCKERR),
              "Request_Exclusive after Request_Exclusive sets LockErr %s",
              tag);
    DECREF(lock1);

    TEST_TRUE(runner, Lock_Request_Shared(lock2),
              "Request_Shared succeeds after Destroy %s", tag);
    DECREF(lock2);
    DECREF(lock3);
}

static void
test_stale(TestBatchRunner *runner, Folder *folder, const char *tag) {
    String *name  = SSTR_WRAP_C("test");
    String *host1 = SSTR_WRAP_C("fuchur");
    String *host2 = SSTR_WRAP_C("drogon");
    LockFileLock *lock1 = LFLock_new(folder, name, host1, 0, 100, false);
    LockFileLock *lock2 = LFLock_new(folder, name, host2, 0, 100, false);
    LockFileLock *tmp;
    Hash *hash;

    tmp = LFLock_new(folder, name, host1, 0, 100, false);
    LFLock_Request_Exclusive(tmp);
    String *ex_path = SSTR_WRAP_C("locks/test.lock");
    hash = (Hash*)Json_slurp_json(folder, ex_path);
    TEST_TRUE(runner, CERTIFY(hash, HASH), "Lock file %s exists %s",
              Str_Get_Ptr8(ex_path), tag);
    DECREF(tmp);
    // Write lock file with different pid.
    Hash_Store(hash, SSTR_WRAP_C("pid"), (Obj*)Str_newf("10000000"));
    Json_spew_json((Obj*)hash, folder, ex_path);
    DECREF(hash);
    TEST_FALSE(runner, LFLock_Request_Exclusive(lock2),
               "Lock_Exclusive fails with other host's exclusive lock %s",
               tag);
    TEST_TRUE(runner, LFLock_Request_Exclusive(lock1),
              "Lock_Exclusive succeeds with stale exclusive lock %s", tag);
    LFLock_Release(lock1);

    tmp = LFLock_new(folder, name, host1, 0, 100, false);
    LFLock_Request_Shared(tmp);
    String *sh_path = SSTR_WRAP_C("locks/test-1.lock");
    hash = (Hash*)Json_slurp_json(folder, sh_path);
    TEST_TRUE(runner, CERTIFY(hash, HASH), "Lock file %s exists %s",
              Str_Get_Ptr8(sh_path), tag);
    DECREF(tmp);
    // Write lock file with different pid.
    Hash_Store(hash, SSTR_WRAP_C("pid"), (Obj*)Str_newf("10000000"));
    Json_spew_json((Obj*)hash, folder, SSTR_WRAP_C("locks/test-98765.lock"));
    DECREF(hash);
    TEST_FALSE(runner, LFLock_Request_Exclusive(lock2),
               "Lock_Exclusive fails with other host's shared lock %s", tag);
    TEST_TRUE(runner, LFLock_Request_Exclusive(lock1),
              "Lock_Exclusive succeeds with stale shared files %s", tag);
    LFLock_Release(lock1);

    DECREF(lock2);
    DECREF(lock1);
}

static void
test_Obtain(TestBatchRunner *runner, Lock *lock1, Lock *lock2,
            const char *tag) {
    TEST_TRUE(runner, Lock_Obtain_Shared(lock1), "Obtain_Shared succeeds %s",
              tag);
    TEST_FALSE(runner, Lock_Obtain_Exclusive(lock2),
               "Obtain_Exclusive after Obtain_Shared fails %s", tag);
    Lock_Release(lock1);

    TEST_TRUE(runner, Lock_Obtain_Exclusive(lock1),
              "Obtain_Exclusive succeeds %s", tag);
    TEST_FALSE(runner, Lock_Obtain_Shared(lock2),
               "Obtain_Shared after Obtain_Exclusive fails %s", tag);
    DECREF(lock2);
    DECREF(lock1);
}

static void
S_try_request_shared(void *context) {
    Lock_Request_Shared((Lock*)context);
}

static void
S_try_request_exclusive(void *context) {
    Lock_Request_Exclusive((Lock*)context);
}

static void
S_try_release(void *context) {
    Lock_Release((Lock*)context);
}

static void
test_double_request_release(TestBatchRunner *runner, Lock *lock,
                            const char *tag) {
    Err *err;

    Lock_Request_Shared(lock);
    err = Err_trap(S_try_request_shared, lock);
    TEST_TRUE(runner, err, "Request_Shared/Request_Shared fails %s",
              tag);
    DECREF(err);
    Lock_Release(lock);

    Lock_Request_Exclusive(lock);
    err = Err_trap(S_try_request_shared, lock);
    TEST_TRUE(runner, err, "Request_Shared/Request_Exclusive fails %s",
              tag);
    DECREF(err);
    Lock_Release(lock);

    Lock_Request_Shared(lock);
    err = Err_trap(S_try_request_exclusive, lock);
    TEST_TRUE(runner, err, "Request_Exclusive/Request_Shared fails %s",
              tag);
    DECREF(err);
    Lock_Release(lock);

    Lock_Request_Exclusive(lock);
    err = Err_trap(S_try_request_exclusive, lock);
    TEST_TRUE(runner, err,
              "Request_Exclusive/Request_Exclusive fails %s", tag);
    DECREF(err);
    Lock_Release(lock);

    err = Err_trap(S_try_release, lock);
    TEST_TRUE(runner, err, "Double Release fails");
    DECREF(err);

    DECREF(lock);
}

static void
test_empty_lock_folder(TestBatchRunner *runner, Folder *folder,
                       const char *name, const char *tag) {
    Vector *files = Folder_List_R(folder, SSTR_WRAP_C("locks"));
    TEST_TRUE(runner, files, "Lock folder exists after %s tests %s", name,
              tag);
    TEST_UINT_EQ(runner, Vec_Get_Size(files), 0,
                 "Lock folder is empty after %s tests %s", name, tag);
    DECREF(files);
}

static void
test_lf_lock_with_folder(TestBatchRunner *runner, Folder *folder,
                         const char *tag) {
    String *name = SSTR_WRAP_C("test");
    String *host = SSTR_WRAP_C("fuchur");
    LockFileLock *lock1, *lock2, *lock3;

    lock1 = LFLock_new(folder, name, host, 0, 100, true);
    lock2 = LFLock_new(folder, name, host, 0, 100, true);
    test_exclusive_only(runner, (Lock*)lock1, (Lock*)lock2, tag);
    test_empty_lock_folder(runner, folder, "exclusive only", tag);

    lock1 = LFLock_new(folder, name, host, 0, 100, false);
    lock2 = LFLock_new(folder, name, host, 0, 100, false);
    lock3 = LFLock_new(folder, name, host, 0, 100, false);
    test_lock(runner, (Lock*)lock1, (Lock*)lock2, (Lock*)lock3, tag);
    test_empty_lock_folder(runner, folder, "lock", tag);

    lock1 = LFLock_new(folder, name, host, 10, 1, false);
    lock2 = LFLock_new(folder, name, host, 10, 1, false);
    test_Obtain(runner, (Lock*)lock1, (Lock*)lock2, tag);
    test_empty_lock_folder(runner, folder, "Obtain", tag);

    lock1 = LFLock_new(folder, name, host, 0, 100, false);
    test_double_request_release(runner, (Lock*)lock1, tag);
    test_empty_lock_folder(runner, folder, "double", tag);

    test_stale(runner, folder, tag);
    test_empty_lock_folder(runner, folder, "stale", tag);
}

static void
test_lf_lock(TestBatchRunner *runner) {
    Folder *fs_folder = S_create_fs_folder();
    test_lf_lock_with_folder(runner, fs_folder, "(FSFolder)");
    S_destroy_fs_folder(fs_folder);

    Folder *ram_folder = (Folder*)RAMFolder_new(NULL);
    test_lf_lock_with_folder(runner, ram_folder, "(RAMFolder)");
    DECREF(ram_folder);
}

void
TestLFLock_Run_IMP(TestLockFileLock *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 82);
    test_lf_lock(runner);
}

