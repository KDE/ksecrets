
#define PAM_SM_AUTH
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#include <syslog.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

/*  TODO create a library for password token handling? or just do it here? */
/*  Also, see this: */
/*  http://stackoverflow.com/questions/14548748/encrypting-a-file-from-a-password-using-libgcrypt */

int kss_can_change_password(pam_handle_t *pamh) {
  /* nothing to do for the moment */
  return PAM_IGNORE;
}

int kss_change_password(
    pam_handle_t *pamh) {
  const char *password;
  password = 0;
  int result = pam_get_item(pamh, PAM_AUTHTOK, (const void**)&password);
  if (result != PAM_SUCCESS) {
    pam_syslog(pamh, LOG_ERR, "Couldn't get password %s",
        pam_strerror(pamh, result));
    return PAM_IGNORE;
  }

  if (0 != password) {
    pam_syslog(pamh, LOG_INFO, "pam_sm_authenticate got the password, going to update kernel keyring.");
  } else {
    pam_syslog(pamh, LOG_WARNING, "pam_sm_authenticate got NULL password! KSecret Service may prompt a password later.");
  }
  return PAM_SUCCESS;
}

int kss_derive_auth_key( pam_handle_t *pamh) {
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_authenticate(
    pam_handle_t *pamh,
    int flags,
    int argc,
    const char **argv) {
  pam_syslog(pamh, LOG_INFO, "pam_sm_authenticate flags=%X, argc=%d", flags, argc);

  return kss_derive_auth_key(pamh);
}

PAM_EXTERN int pam_sm_setcred(
    pam_handle_t *pamh,
    int flags,
    int argc,
    const char **argv) {
  pam_syslog(pamh, LOG_INFO, "pam_sm_setcred flags=%X, argc=%d", flags, argc);
  return PAM_IGNORE;
}

PAM_EXTERN int pam_sm_open_session(
    pam_handle_t *pamh,
    int flags,
    int argc,
    const char **argv) {
  pam_syslog(pamh, LOG_INFO, "pam_sm_open_session flags=%X, argc=%d", flags, argc);
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(
    pam_handle_t *pamh,
    int flags,
    int argc,
    const char **argv) {
  pam_syslog(pamh, LOG_INFO, "pam_sm_close_session flags=%X, argc=%d", flags, argc);
  return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_chauthtok(
    pam_handle_t *pamh,
    int flags,
    int argc,
    const char **argv) {
  syslog(LOG_WARNING, "pam_sm_chauthtok");
  pam_syslog(pamh, LOG_INFO, "pam_sm_chauthtok flags=%X, argc=%d", flags, argc);

  if (flags & PAM_PRELIM_CHECK) {
    pam_syslog(pamh, LOG_INFO, "pam_sm_chauthtok preliminary check");
    return kss_can_change_password(pamh);
  }
  if (flags & PAM_UPDATE_AUTHTOK) {
    pam_syslog(pamh, LOG_INFO, "pam_sm_chauthtok attempt updating ksecret service key");
    /* TODO here we should rebuild the secrets file */
    /* TODO the unlocking routine should also handle the case where the module */
    /* failed to update the passwd, e.g. it should ask the old password */
    return kss_change_password(pamh);
  }

  return PAM_IGNORE;
}
