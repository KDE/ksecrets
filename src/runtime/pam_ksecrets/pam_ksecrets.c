
#define PAM_SM_AUTH
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <ksecrets_credentials.h>

#include <syslog.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <linux/limits.h>
#include <string.h>

#define UNUSED(x) (void)(x)

const char* password;

PAM_EXTERN int pam_sm_authenticate(
    pam_handle_t* pamh, int flags, int argc, const char** argv)
{
    pam_syslog(pamh, LOG_INFO, "pam_sm_authenticate flags=%X", flags);
    UNUSED(flags);
    UNUSED(argc);
    UNUSED(argv);

    password = 0;
    int result = pam_get_item(pamh, PAM_AUTHTOK, (const void**)&password);
    if (result != PAM_SUCCESS) {
        pam_syslog(pamh, LOG_ERR, "Couldn't get password %s",
            pam_strerror(pamh, result));
    }

    /* this module does not participate to the user authentication process */
    return PAM_IGNORE;
}

/**
 * The module PAM module configuration should specify the location of the
 * secrets file. The location should contain only the part of the path
 * relative to the user's $HOME directory. So, if you'd like to have secrets
 * managed in, say, $HOME/owncloud/ksecrets.data then the pam configuration
 * file should contain something like this:
 *
 * auth required pam_ksecrets.so owncloud/ksecrets.data
 *
 * As you may have guessed, the configuration will be the same for all the
 * users, and that cannot be changed from one user to another, at least not in
 * this version of pam_ksecrets.
 *
 * If nothing is specified, then the default path will be
 * $HOME/.local/share/ksecrets/ksecrets.data
 *
 * FIXME see how this could be simplified or how one could add a configuration
 * file handling here. Handling configuration files is DE specific and this
 * pam module tries to stay as generic as possible. Perhaps we could add here
 * a DE-specific plugin that would retrieve values from the DE-specific configuration
 * files, using the DE-specific configuration handling libraries.
 *
 * The location should point to an actual file. If it's a symlink, then the
 * store handling routine will fail.
 */
PAM_EXTERN int pam_sm_setcred(
    pam_handle_t* pamh, int flags, int argc, const char** argv)
{
    UNUSED(argc);
    UNUSED(argv);
    pam_syslog(pamh, LOG_INFO, "pam_sm_setcred flags=%X", flags);
    if (flags & PAM_ESTABLISH_CRED) {
        if (0 == password)
            return PAM_CRED_UNAVAIL;

        const char* user_name;
        user_name = 0;
        int result = pam_get_item(pamh, PAM_USER, (const void**)&user_name);
        if (result != PAM_SUCCESS) {
            pam_syslog(pamh, LOG_ERR, "Couldn't get password %s",
                pam_strerror(pamh, result));
            return PAM_CRED_UNAVAIL;
        }

        struct passwd *pwd;
        pwd = getpwnam(user_name);
        if (pwd == 0) {
            pam_syslog(pamh, LOG_ERR, "Couldn't get user passwd info %d (%m)", errno);
            return PAM_CRED_ERR;
        }

        char secrets_path[PATH_MAX];
        memset(secrets_path, 0, sizeof(secrets_path)/sizeof(secrets_path[0]));
        strncpy(secrets_path, pwd->pw_dir, PATH_MAX);
        static const char *defaultPath = ".local/share/ksecrets/ksecrets.data";
        if (argc == 1 && argv[0] != 0) {
            strncat(secrets_path, argv[0], PATH_MAX - strlen(secrets_path) -1);
        } else {
            strncat(secrets_path, defaultPath, PATH_MAX - strlen(secrets_path) -1);
        }
        pam_syslog(pamh, LOG_INFO, "ksecrets: setting secrets path to %s", secrets_path);

        if (!kss_set_credentials(user_name, password, secrets_path)) {
            pam_syslog(
                pamh, LOG_ERR, "ksecrets credentials could not be set.");
            return PAM_CRED_ERR;
        }
        return PAM_SUCCESS;
    }
    if (flags & PAM_DELETE_CRED) {
        kss_delete_credentials();
    }
    return PAM_IGNORE;
}

PAM_EXTERN int pam_sm_open_session(
    pam_handle_t* pamh, int flags, int argc, const char** argv)
{
    UNUSED(pamh);
    UNUSED(flags);
    UNUSED(argc);
    UNUSED(argv);
    /* not used */
    return PAM_IGNORE;
}

PAM_EXTERN int pam_sm_close_session(
    pam_handle_t* pamh, int flags, int argc, const char** argv)
{
    UNUSED(pamh);
    UNUSED(flags);
    UNUSED(argc);
    UNUSED(argv);
    /* not used */
    return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_chauthtok(
    pam_handle_t* pamh, int flags, int argc, const char** argv)
{
    UNUSED(argc);
    UNUSED(argv);
    pam_syslog(pamh, LOG_INFO, "pam_sm_chauthtok flags=%X", flags);

    if (flags & PAM_PRELIM_CHECK) {
        pam_syslog(pamh, LOG_INFO, "pam_sm_chauthtok preliminary check");
        if (kss_can_change_password()) {
            return PAM_SUCCESS;
        }
        else {
            pam_syslog(pamh, LOG_ERR,
                "pam_sm_chauthtok prelimnary check failed "
                "because ksecrets cannot be locked");
            return PAM_AUTHTOK_LOCK_BUSY;
        }
    }

    if (flags & PAM_UPDATE_AUTHTOK) {
        pam_syslog(pamh, LOG_INFO,
            "pam_sm_chauthtok attempt updating ksecret service key");

        const char* password;
        password = 0;
        int result = pam_get_item(pamh, PAM_AUTHTOK, (const void**)&password);
        if (result != PAM_SUCCESS) {
            pam_syslog(pamh, LOG_ERR, "Couldn't get password %s",
                pam_strerror(pamh, result));
            return PAM_IGNORE;
        }

        if (0 == password) {
            pam_syslog(
                pamh, LOG_WARNING, "pam_sm_authenticate got NULL password! ");
            return PAM_AUTHTOK_ERR;
        }

        if (kss_change_password(password))
            return PAM_SUCCESS;
        else {
            pam_syslog(pamh, LOG_ERR,
                "ksecrets service failed to update the keys. "
                "Aborting password change.");
            return PAM_AUTHTOK_ERR;
        }
    }

    return PAM_IGNORE;
}
