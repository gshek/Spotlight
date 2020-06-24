# Tests

## Pre-requisites
* Ensure [requirements](../README.md) are satisfied for building the source.
* Bash

## Running Tests

```bash
make
```

This skips the slow tests and takes around 5 minutes to complete.

## Running Extended Tests

```bash
make test-all
```

It is important to note that running all tests take a very long time
(> 10 hours). It is currently unknown why it takes so long for these tests to
complete.

# Acknowledgements

* [ext4fuse](https://github.com/gerard/ext4fuse/blob/master/test/lib.sh) - some
tests have been derived from the tests in ext4fuse.
