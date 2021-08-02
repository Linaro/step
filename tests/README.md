# Secure Telemetry Pipeline (STeP) Unit Tests

This folder contains unit tests for the secure telemetry pipeline library.

## Running tests via `twister`

To run these units tests, make sure that you have previously run
`source zephyr-env.sh` (or it's equivalent for your system), and
execute the following command to start the test-runner:

```
$ cd $ZEPHYR_BASE
$ ./scripts/twister --inline-logs -p qemu_cortex_m3 -T ../modules/lib/step/tests
```

> **Tip**: If you wish to have **verbose output** to see any errors in
  the test output if you make changes, you can append the `-vv` flag.

## File structure

Unit tests are broken up into logical groups that correspond to the source
files in this library. When adding new tests, please try to respect the current
structure so that unit tests remain logically organised and easy to find.
