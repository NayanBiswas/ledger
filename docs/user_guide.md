# User Guide

## Disk Storage

Ledger writes all data under `/data/ledger`. In order for data to be persisted,
ensure that a persistent partition is mounted under /data. See [minfs
setup](https://fuchsia.googlesource.com/magenta/+/master/docs/minfs.md).

## Cloud Sync

By default Ledger runs without sync. To enable sync, follow the instructions
below.

When enabled, Cloud Sync synchronizes the content of all pages using the
selected cloud provider (see below for configuration.

**Note**: Ledger does not support **migrations** between cloud providers. Once
Cloud Sync is set up, you won't be able to switch to a different cloud provider
(e.g. to a different Firebase instance) without flushing the locally persisted
data.

### Prerequisities

Follow the instructions in
[netstack](https://fuchsia.googlesource.com/netstack/+/master/README.md) to
ensure that your Fuchsia has Internet access.

Run `wget` to verify that it worked:

```
@ bootstrap /system/test/wget http://example.com
```

You should see the HTML content of the `example.com` placeholder page.

### Setup

To use sync, you will need an instance of the Firebase Real-Time Database. You
can get one at https://firebase.google.com/.

Ledger does not currently support authorization, so the database needs to be
public-readable and public-writable (better not to store anything private
there). You also need to **set up indexes** that allow Ledger to perform queries
on synced metadata. Go to the [Firebase
Console](https://console.firebase.google.com/) and set the following in
`Database / Rules`:

```
{
  "rules": {
    ".read": true,
    ".write": true,
    "$user": {
      "$version": {
        "$app": {
          "$page": {
            "commits": {
              ".indexOn": ["timestamp"]
            }
          }
        }
      }
    }
  }
}
```

In order to point Ledger to your database, run the configuration script:

```
configure_ledger --firebase_id=<DATABASE_ID> --user_prefix=<USER_IDENTITY>
```

`DATABASE_ID` is the identifier of your Firebase project. (it's "ABC" for a
firebase database "ABC.firebaseio.com")

`USER_IDENTITY` is a stop-gap self-declared identity of the user. You can share
one instance of the database between multiple users declaring different
identities.

### Diagnose

If something seems off with sync, run the following command:

```
@ bootstrap cloud_sync doctor
```

If the provided information is not enough to resolve the problem, please file a
bug and attach the output.

### Toggling Cloud Sync off and on

To disable sync:

```
configure_ledger --nosync
```

To enable it again:

```
configure_ledger --sync
```

The configuration tool remembers the Firebase settings even when the sync is
off, so you don't need to pass them again.
