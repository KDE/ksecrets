
#define PAM_SM_AUTH
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <ksecrets_credentials.h>

#include <syslog.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#define UNUSED(x) (void)(x)

const char* password;

/* these extern functions are implemented in ksecrets_store_bridge.cpp */
extern int kss_set_credentials(const char*, const char*);
extern int kss_delete_credentials();
extern int kss_can_change_password();
extern int kss_change_password(const char*);

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
    }
    if (!kss_set_credentials(user_name, password)) {
      pam_syslog(pamh, LOG_ERR, "ksecrets credentials could not be set.");
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
      pam_syslog(pamh, LOG_ERR, "pam_sm_chauthtok prelimnary check failed "
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
      pam_syslog(pamh, LOG_ERR, "ksecrets service failed to update the keys. "
                                "Aborting password change.");
      return PAM_AUTHTOK_ERR;
    }
  }

  return PAM_IGNORE;
}
