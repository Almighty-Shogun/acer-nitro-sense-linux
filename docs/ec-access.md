# EC Access

The daemon can use three Embedded Controller access paths:

1. `ec_sys`: `/sys/kernel/debug/ec/ec0/io`
2. `acpi_ec`: `/dev/ec`
3. direct x86 EC I/O ports: `0x66/0x62`

The service tries to preload `ec_sys` and `acpi_ec`, but both module loads are
best-effort. The daemon then probes each backend in order.

## Fedora

Normal Fedora kernels generally do not provide `ec_sys` because
`CONFIG_ACPI_EC_DEBUGFS` is not enabled for standard kernels. Fedora has kept
that interface out of normal kernels because it is a debug interface for direct
EC access.

With Secure Boot enabled, kernel lockdown blocks direct userspace I/O port
access through `ioperm`, even for root. In that setup there is no userspace-only
bypass for this project: the realistic options are:

- use the external `acpi_ec` DKMS module, signed through MOK when Secure Boot is
  enabled
- disable Secure Boot or kernel lockdown, then use the direct I/O fallback
- boot a Fedora debug kernel that provides `ec_sys`

The first option is the least disruptive for normal Fedora installs.

## Fake EC

A fake EC backend is useful for tests, screenshots, companion UI testing, and
CI. It cannot replace real EC access for fan control because the physical fan
registers still live behind kernel-mediated hardware access.

Use a model with:

```json
{
  "ec": {
    "path": "fake"
  }
}
```

The repository includes `tests/fixtures/fake-model.json`. Fake RPM registers are
seeded with deterministic CPU/GPU values. Fake temperatures can be supplied with:

```sh
ANS_FAKE_CPU_TEMP_C=55 ANS_FAKE_GPU_TEMP_C=50 \
  build/acer-nitro-sensed --config tests/fixtures/fake-model.json --force-model --probe
```

Daemon behavior tests are available through:

```sh
build/acer-nitro-sense-unit-test
meson test -C build
```

The unit-test covers command parsing, command execution, authorization denial,
safety clamps, global critical overrides, preset writes through fake EC,
resume-style reapply, and status response format. Meson also checks valid model
fixtures and rejects malformed profile fixtures.

To avoid `acpi_ec` on locked-down Fedora, this project would need one of:

- a kernel that enables `ec_sys`
- a signed kernel module exposing `/dev/ec`
- disabled kernel lockdown so direct I/O ports are available

Writing our own in-tree "fake EC" would only simulate fan data. Writing our own
real EC kernel module would have the same Secure Boot signing/MOK problem as
`acpi_ec`, plus a larger maintenance and safety burden.

## Install `acpi_ec` On Fedora

```sh
sudo dnf install kernel-devel dkms make openssl mokutil
git clone https://github.com/MusiKid/acpi_ec /tmp/acpi_ec
cd /tmp/acpi_ec
sudo ./install.sh
```

If Secure Boot is enabled, import the DKMS signing key:

```sh
sudo mokutil --import /var/lib/dkms/mok.pub
```

Reboot and choose:

```text
Enroll MOK
Continue
Yes
<one-time password>
Reboot
```

## Verify

```sh
sudo modprobe acpi_ec
ls -l /dev/ec
sudo acer-nitro-sensed --config /etc/acer-nitro-sense/model.json --probe
```

Expected backend:

```text
backend=acpi-ec
```

## References

- Fedora bug 1943318: `ec_sys` intentionally missing from normal Fedora kernels:
  https://bugzilla.redhat.com/show_bug.cgi?id=1943318
- Linux `CONFIG_ACPI_EC_DEBUGFS` help text:
  https://cateee.net/lkddb/web-lkddb/ACPI_EC_DEBUGFS.html
- Linux kernel lockdown restrictions:
  https://manpages.ubuntu.com/manpages/noble/man7/kernel_lockdown.7.html
