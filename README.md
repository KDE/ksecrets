# KDE Secrets Service

This framework contains all the necessary tools needed to securely store and handle application secrets.

## Introduction

KDE applications use this framework to securely store and handle appliction
secrets. The storage could be a KDE specific one, or some other storage that
complies to the freedesktop Secret Service specification available here.

http://standards.freedesktop.org/secret-service/

The Free Desktop draft would also instruct you how to configure the storage
backend in order to activate the one you prefere - ksecrets or some other
storage like gnome-keyring.

This documentation only covers the KDE specific code.

The main components of the KSecrets Service are:

* pam_ksecrets

* KF5::Secrets framework

* ksecrets

TODO - write more documentation here. However, right now I'd like to get it working ;-)

