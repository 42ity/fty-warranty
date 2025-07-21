# fty-warranty

Agent producing _end-warranty-date_ metrics in the shared memory.

## How to build

To build, run:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=usr -DBUILD_TESTING=On ..
make
sudo make install
```

## How to run

To run:

* from within the build/ tree, run:

```bash
./fty-warranty
```

For the other options available, refer to the manual page of fty-alert-engine

* from an installed base, using systemd, run:

```bash
systemctl start fty-warranty
```

### Configuration file

__IGNORED__ Configuration file - fty-warranty.cfg.
