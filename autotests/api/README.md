= Setting-up KSecrets Autotests

KSecrets service needs decrpytion keys to be present into the current kernel
keyring. These keys are normally put there by the pam_ksecrets module.
That module won't be present in a test environment. However, the underlying
code is able to function into a test environment. Just follow this steps:

. Create a test user into the test system. That's because the tests use
/etc/passwd information. This test user should also have a valid home
directory where the CI user could write. For example, let the name of the user
be "ksstest"

. Define a password for that user. For example, let it be "ksstest".

. Launch the test

  su - ksstest -c "export KSS_TEST_PASSWORD=ksstest && $PWD/autotests/api/ksecretsservice-test"

. That's it!

